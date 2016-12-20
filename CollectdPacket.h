// This file is part of CollectdPacket library.
//
// CollectdPacket is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// CollectdPacket is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with CollectdPacket.  If not, see <http://www.gnu.org/licenses/>

// 2015-03-24 David Goncalves - Version 1.0.0
//            Abstraction class to create a collectd packet suitable to be sent
//            to a running collectd server.

#include "Arduino.h"

#ifndef CollectdPacket_h
#define CollectdPacket_h

#define VALUE_TYPE_COUNTER  0x00
#define VALUE_TYPE_GAUGE    0x01
#define VALUE_TYPE_DERIVE   0x02
#define VALUE_TYPE_ABSOLUTE 0x03

class CollectdPacket
{
    public:
        CollectdPacket( const char *hostname, unsigned long interval );
        void addPlugin( const char *plugin );
        void addPluginInstance( const char *instance );
        void addType( const char *type );
        void addTypeInstance( const char *instance );
        void addTimestamp( unsigned long timestamp );
        void addTimestampHR( unsigned long timestamp );
        void addValue( byte type, float value );
        
        word getPacketSize( void );
        byte *getPacketBuffer( void );
        void resetPacket( void );
        
    private:
        byte buffer[ 256 ];
        word offset;
        
        void add_string( word packet_id, const char *string );
        void add_numeric( word packet_id, unsigned long value );
        void add_numeric_hr( word packet_id, unsigned long value );
};

#endif
