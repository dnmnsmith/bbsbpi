
#ifndef BYE_BYE_STANDBY_SENDER
#define BYE_BYE_STANDBY_SENDER

class ByeByeStandby
{
 typedef unsigned char byte;
 
 public:
        ByeByeStandby();
        virtual ~ByeByeStandby();
        
        void init( int outputPin );
        
        void sendSimpleProtocolMessage(unsigned int s, unsigned int r, bool c);
        void sendAdvancedProtocolMessage(unsigned long s, unsigned int r, bool c, bool g);
        void sendBBSB2011ProtocolMessage(unsigned int s, unsigned int r, bool c, bool g); 

        // Number of times to re-transmit message, default 5.
        void setTramitterMessageCount( int count );

private:
        void reset();
        void setSignal( bool on );
        void finishSendMessage();
        void initSending();
        void sendMessage();

        // variables for sending messages
        int m_outputPin;

        unsigned long sender;
        unsigned int recipient;
        byte command;
        bool group;
        
        byte messageType;
        unsigned int messageCount;
        unsigned int pulseWidth;
        unsigned int latchStage;
        bool bbsb2011;
        signed int bitCount;
        byte bit;
        byte prevBit;

        int transmitter_message_count;

        enum { 
             MESSAGE_TYPE_SIMPLE = 0, 
             MESSAGE_TYPE_ADVANCED = 1,
             MESSAGE_TYPE_BBSB2011 = 2 };
        
};

#endif
