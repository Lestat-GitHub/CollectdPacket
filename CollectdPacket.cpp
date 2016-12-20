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
#include "CollectdPacket.h"
#include "IEEE754tools.h"

CollectdPacket::CollectdPacket( const char *hostname, unsigned long interval )
{
    // The class constructor
    this->offset = 0;
    memset( this->buffer, 0, sizeof( this->buffer ) );
    
    this->add_string( 0x0000, hostname );
    
    // This is the old format before v5
    //this->add_numeric( 0x0007, interval );
    
    // New format uses high resolution time
    this->add_numeric_hr( 0x0009, interval );
}

void CollectdPacket::add_string( word packet_id, const char *string )
{
    word len = strlen( string ) + 1;  // Add the null byte at the end of the string
    
    // Write the packet type and length to buffer (including 2 bytes for type and 2 for length)
    this->buffer[ this->offset++ ] = highByte( packet_id );
    this->buffer[ this->offset++ ] = lowByte( packet_id );
    this->buffer[ this->offset++ ] = highByte( len + 4 );
    this->buffer[ this->offset++ ] = lowByte( len + 4 );
    
    // Write the string to buffer at "buffer address + offset"
    strcpy( (char *) this->buffer + this->offset, string );
    this->offset += len;
}

void CollectdPacket::add_numeric( word packet_id, unsigned long value )
{
    const word len = 12;   // Numeric packet lenght is always 12
    
    // Write the packet type and length to buffer (including 2 bytes for type and 2 for length)
    this->buffer[ this->offset++ ] = highByte( packet_id );
    this->buffer[ this->offset++ ] = lowByte( packet_id );
    this->buffer[ this->offset++ ] = highByte( len );
    this->buffer[ this->offset++ ] = lowByte( len );
    
    // Write 4 bytes of zero as we are using 32 bit only values
    memset( this->buffer + this->offset, 0, 4 );
    this->offset += 4;
    
    this->buffer[ this->offset++ ] = value >> 24;
    this->buffer[ this->offset++ ] = value >> 16;
    this->buffer[ this->offset++ ] = value >>  8;
    this->buffer[ this->offset++ ] = (byte) value;
}

void CollectdPacket::add_numeric_hr( word packet_id, unsigned long value )
{
    word     len       = 12;
    uint64_t inthr     = value * 1073741824LL;

    this->buffer[ this->offset++ ] = highByte( packet_id );
    this->buffer[ this->offset++ ] = lowByte( packet_id );
    this->buffer[ this->offset++ ] = highByte( len );
    this->buffer[ this->offset++ ] = lowByte( len );
    
    // Copy bytes in reverse order to match the Endian coding
    for ( byte i=0; i<8; i++ )
    {
        this->buffer[ this->offset++ ] = inthr >> ( 56 - ( 8 * i ) );
    }    
}

void CollectdPacket::addPlugin( const char *plugin )
{
    this->add_string( 0x0002, plugin );
}

void CollectdPacket::addPluginInstance( const char *instance )
{
    this->add_string( 0x0003, instance );
}

void CollectdPacket::addType( const char *type )
{
    this->add_string( 0x0004, type );
}

void CollectdPacket::addTypeInstance( const char *instance )
{
    this->add_string( 0x0005, instance );
}

void CollectdPacket::addTimestamp( unsigned long timestamp )
{
    this->add_numeric( 0x0001, timestamp );
}

void CollectdPacket::addTimestampHR( unsigned long timestamp )
{
    // Add a timestamp in high resolution format expressed in 2^-30 seconds, i.e. in 0.00000000093132257461 second steps
    // According to collectd documentation, a simple 30 bit left shift would convert
    // a classical timestamp to high resolution
    // New format uses high resolution time
    
    this->add_numeric_hr( 0x0008, timestamp );
}

void CollectdPacket::addValue( byte type, float value )
{
    // Adds a single value to the Collectd packet
    word packet_id = 0x0006;
    word len       = 6 + 1 + 8; // Headers + data type + data (64bit = 8 bytes)
    
    static byte float_buffer[8];
    
    // Add the packet ID
    this->buffer[ this->offset++ ] = highByte( packet_id );
    this->buffer[ this->offset++ ] = lowByte( packet_id );

    // Add the packet length    
    this->buffer[ this->offset++ ] = highByte( len );
    this->buffer[ this->offset++ ] = lowByte( len );
    
    // Number of values
    this->buffer[ this->offset++ ] = highByte( (word) 1 );
    this->buffer[ this->offset++ ] = lowByte( (word) 1 );
    
    // Data type
    this->buffer[ this->offset++ ] = type;
    
    // The value
    // Convert the float to a 64 bit representation in a byte array as
    // collectd uses 64 bit data.    
    float2DoublePacked( value, float_buffer );
    memcpy( this->buffer + this->offset, float_buffer, 8 );
    this->offset += 8;
}

word CollectdPacket::getPacketSize( void )
{
    return( this->offset );    
}

byte *CollectdPacket::getPacketBuffer( void )
{
    return( this->buffer );
}

void CollectdPacket::resetPacket( void )
{
    // The reset function is useful when you want to use the class in a loop. This function will
    // reset all values except the 'hostname' and the 'interval'. You can then re-add data types
    // on the packet to send it again.

    // Find the offset just after the hostname + interval (12 bytes)
    word offset = ( ( this->buffer[ 2 ] << 8 ) + this->buffer[ 3 ] ) + 12;
    
    // Now clear all following bytes until 1024
    this->offset = offset;
    memset( this->buffer + offset, 0, ( sizeof( this->buffer ) - offset ) );
}
