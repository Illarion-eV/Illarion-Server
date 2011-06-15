#include "Item.hpp"
#include <sstream>

uint32_t Item::getData() const 
{
    DATA_MAP::const_iterator it = data_map.find(0);
    if ( it == data_map.end() )return 0;
    std::istringstream myStream(it->second);
    uint32_t ret;
    if (myStream>>ret)return ret;
    else return 0;
}

    
void Item::setData(uint32_t newdata)
{
    std::stringstream stream;
    stream << newdata;
    data_map[0] = stream.str();
}

std::string Item::getValue(uint8_t key)
{
    return data_map[key];
}


void Item::setValue(uint8_t key,std::string val)
{
    data_map[key] = val;
}


void Item::save(std::ostream* obj)
{
    obj->write( ( char* ) &id, sizeof( TYPE_OF_ITEM_ID ) );
    obj->write( ( char* ) &number, sizeof( unsigned char ) );
    obj->write( ( char* ) &wear, sizeof( unsigned char ) );
    obj->write( ( char* ) &quality, sizeof( uint16_t ) );
    obj->write( ( char* ) &data, sizeof( uint32_t ) );
    uint8_t mapsize = static_cast<uint8_t>(data_map.size());
    obj->write( ( char* ) &mapsize, sizeof( uint8_t ) );
    for ( DATA_MAP::iterator it = data_map.begin(); it != data_map.end(); ++it)
    {
        //uint8_t sz = static_cast<uint8_t>(it->first.size());
        //obj->write( ( char* ) &sz , sizeof( uint8_t ) );
        obj->write( ( char* ) &it->first , sizeof( uint8_t ) );
        uint8_t sz2 = static_cast<uint8_t>(it->second.size());
        obj->write( ( char* ) &sz2 , sizeof( uint8_t ) );
        obj->write( ( char* ) it->second.data() , sz2 );
    }
}

void Item::load(std::istream* obj)
{
    obj->read( ( char* ) &id, sizeof( TYPE_OF_ITEM_ID ) );
    obj->read( ( char* ) &number, sizeof( unsigned char ) );
    obj->read( ( char* ) &wear, sizeof( unsigned char ) );
    obj->read( ( char* ) &quality, sizeof( uint16_t ) );
    obj->read( ( char* ) &data, sizeof( uint32_t ) );
    uint8_t tempsize;
    obj->read( ( char* ) &tempsize, sizeof( uint8_t ) );
    char readStr[255];
    for ( int i = 0; i < tempsize; ++i)
    {
        uint8_t sz = 0;
        //obj->read( (char*) &sz , sizeof( uint8_t ) );
        //obj->read( (char*) readStr, sz );
        //std::string key(readStr,sz);
        uint8_t key;
        obj->read( (char*) &key, sizeof( uint8_t ) );
        obj->read( (char*) &sz , sizeof( uint8_t ) );
        obj->read( (char*) readStr, sz );
        std::string value(key,sz);
        data_map[key] = value;
    }
}
