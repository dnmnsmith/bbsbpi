#include <wiringPi.h>
#include "bbsb.h"


// Other definitions
static const int DEFAULT_TRANSMITTER_MESSAGE_COUNT = 5;

// From Arduino library.
#ifndef _BV
#define _BV(bit)(1 << (bit)) 
#endif

ByeByeStandby::ByeByeStandby( ) :
   m_outputPin( 0 ),
   sender (0),
   recipient(0),
   command(0),
   group(false),
   messageType( MESSAGE_TYPE_SIMPLE ),
   messageCount( 0 ),
   pulseWidth( 10000 ),
   latchStage( 0 ),
   bbsb2011( false ),
   bitCount( 0 ),
   bit( 0 ),
   prevBit( 0 ),
   transmitter_message_count( DEFAULT_TRANSMITTER_MESSAGE_COUNT )
{


}


ByeByeStandby::~ByeByeStandby()
{


}

void ByeByeStandby::init(int outputPin)
{
  m_outputPin = outputPin;
  
  wiringPiSetup() ;   

  pinMode( m_outputPin, OUTPUT );
}

// Arduino would use this to start the interrupts used to send the message
// Raspi will just send the message.
void ByeByeStandby::initSending()
{
 sendMessage();
}

void ByeByeStandby::setTramitterMessageCount( int count )
{
 transmitter_message_count = count;
}


void ByeByeStandby::reset()
{
	// reset variables
	messageCount = 0;
	latchStage = 0;
	bitCount = 0;
	bit = 0;
	prevBit = 0;
	pulseWidth = 10000;
}

/**
 *
 */
void ByeByeStandby::sendSimpleProtocolMessage(unsigned int s, unsigned int r, bool c)
{
	// reset variables
 reset();
	
	// set data to transmit
	sender = s;
	recipient = r;
	command = (c ? 14 : 6);
	
	// specify encoding
	messageType = MESSAGE_TYPE_SIMPLE;
	
	// start the timer interrupt
	initSending();
}

/**
 *
 */
void ByeByeStandby::sendAdvancedProtocolMessage(unsigned long s, unsigned int r, bool c, bool g)
{
	// reset variables
  reset();
  	
	// set data to transmit
	sender = s;
	recipient = r;
	command = c;
	group = g;
	
	// specify encoding
	messageType = MESSAGE_TYPE_ADVANCED;
	
	// start the timer interrupt
	initSending();
}

/**
 *
 */
void ByeByeStandby::sendBBSB2011ProtocolMessage(unsigned int s, unsigned int r, bool c, bool g) 
{
  // reset variables
  reset();
  
  // set data to transmit
  sender = s;
  if (g) {
    recipient = 1;
  } else if (r & 0x4) {
    recipient = 14 - (2 * r);
  } else {
    recipient = 9 - (2 * r);
  }
  command = c;

  // specify encoding
  messageType = MESSAGE_TYPE_BBSB2011;

  // start the timer interrupt
  initSending();
}

void ByeByeStandby::setSignal( bool on )
{
 digitalWrite( m_outputPin, on ? HIGH : LOW );
}

void ByeByeStandby::finishSendMessage()
{
 // Tidyup
				messageCount = 0;
				
        setSignal( false );
        
//				HE_TCCRA = 0x00;
//				HE_TCCRB = 0x02;
//				HE_TIMSK = _BV(HE_ICIE);
}



/**
 * The timer interrupt handler.
 * 
 * This is where the message is transmitted.
 * 
 * The timer interrupt is used to wait for the required length of time.  Each call of this
 * function toggles the output and determines the length of the time until the function is
 * called again.
 * 
 * Once the message has been transmitted this class will switch back to receiving.
 */
void ByeByeStandby::sendMessage()
{
 while (true)
 {
	if(messageType == MESSAGE_TYPE_SIMPLE)
	{
		if(!prevBit && bitCount != 25)
		{
     setSignal( true );
		}
		else
		{
     setSignal( false );
		}
		
		if(bitCount % 2 == 0)
		{	// every other bit is a zero
			bit = 0;
		}
		else if(bitCount < 8)
		{	// sender
			bit = ((sender & _BV((bitCount - 1) / 2)) != 0);
		}
		else if(bitCount < 16)
		{	// recipient
			bit = ((recipient & _BV((bitCount - 9) / 2)) != 0);
		}
		else if(bitCount < 24)
		{	// command
			bit = ((command & _BV((bitCount - 17) / 2)) != 0);
		}
		
		if(bitCount == 25)
		{	// message finished
			
			bitCount = 0;
			messageCount++;
			
			pulseWidth = 10000;
			
			if(messageCount == transmitter_message_count)
			{	// go back to receiving
				finishSendMessage();
				return;
			}
		}
		else
		{
			if(prevBit && bit || !prevBit && !bit)
			{
				pulseWidth = 375;
			}
			else
			{
				pulseWidth = 1125;
			}
			
			if(prevBit)
			{
				bitCount++;
			}
			
			prevBit = !prevBit;
		}
	}
	else if(messageType == MESSAGE_TYPE_ADVANCED)
	{
		if(!prevBit)
		{
      setSignal( true );
		}
		else
		{
      setSignal( false );
		}
		
		if(!prevBit)
		{
			if(bitCount % 2 == 1 || latchStage == 0)
			{	// every other bit is inverted
				bit = !bit;
			}
			else if(bitCount < 52)
			{	// sender
				bit = (((sender << (bitCount / 2)) & 0x02000000) != 0);
			}
			else if(bitCount < 54)
			{	// group
				bit = group;
			}
			else if(bitCount < 56)
			{	// command
				bit = command;
			}
			else if(bitCount < 64)
			{	// recipient
				bit = ((recipient & _BV(31 - (bitCount / 2))) != 0);
			}
		}
		else
		{
			if(latchStage == 1)
			{
				bitCount++;
			}
		}
		
		if(!prevBit)
		{
			pulseWidth = 235;
		}
		else if(latchStage == 0)
		{
			pulseWidth = 2650;
			
			latchStage = 1;
		}
		else if(bitCount > 64)
		{	// message finished
			
			messageCount++;
			
			pulseWidth = 10000;
			latchStage = 0;
			bitCount = 0;
		}
		else if(bit)
		{
			pulseWidth = 1180;
		}
		else
		{
			pulseWidth = 275;
		}
		
		prevBit = !prevBit;

		if(messageCount == transmitter_message_count)
		{	// go back to receiving
			
			finishSendMessage();
 			return;
		}
	}
	else if(messageType == MESSAGE_TYPE_BBSB2011)
	{
		if(!prevBit && bitCount != 25)
		{
       setSignal( true );
		}
		else
		{
        setSignal( false );
		}
		
		if(bitCount < 16)
		{	// sender
			bit = (((sender << bitCount) & 0x8000) != 0);
		}
		else if(bitCount < 20)
		{ // 1010
			bit = ((0x5 & _BV(bitCount - 16)) != 0);
		}
		else if(bitCount < 21)
		{	// command
			bit = command;
		}
		else if(bitCount < 24)
		{	// recipient
			bit = (((recipient << (bitCount - 21)) & 0x4) != 0);
		}
		else if(bitCount < 25)
		{ // 0
			bit = 0;
		}
		
		if(bitCount == 25)
		{	// message finished
			bitCount = 0;
			messageCount++;
			
			pulseWidth = 10000;
			
			if(messageCount == transmitter_message_count)
			{	// go back to receiving
					finishSendMessage();
				  return;
			}
		}
		else
		{
			if(prevBit && bit || !prevBit && !bit)
			{
				pulseWidth = 300;
			}
			else
			{
				pulseWidth = 900;
			}
			
			if(prevBit)
			{
				bitCount++;
			}
			
			prevBit = !prevBit;
		}
	}
	
	// set the next delay
	//HE_OCRA = (pulseWidth * 2);

  // Original arduino code was able to schedule an interrupt after the required delay.
  // Something much uglier required on RPi
  delayMicroseconds( pulseWidth );
  
  }
}

