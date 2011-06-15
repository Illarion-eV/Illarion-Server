#ifndef __FUSE_PTR_HPP__
#define __FUSE_PTR_HPP__

#include <map>
//#include <iostream>
#include <stdexcept>

template<class T>
class fuse_ptr
{
    private:
        // pointer to the object
        T** ptr;
        
        // container for all references from pointers to associated fuse_ptrs
        static std::multimap<T*,fuse_ptr<T> * > fusebox;

    public:
        // contructor for null pointer
        fuse_ptr()
        {
            ptr = new T*;
            *ptr = 0;
        }
        
        // constructor for objects
        fuse_ptr( T* p )
        {
            ptr = new T*;
            *ptr = p;
            //std::cout << this << " is now fuse for " << *ptr << std::endl;
            fusebox.insert( std::pair<T*,fuse_ptr<T> * >(p,this) );
        }

        // copy constructor
        fuse_ptr( fuse_ptr const & p )
        {
            ptr = new T*;
            *ptr = *(p.ptr);
            //std::cout << this << " is now fuse for " << *ptr << std::endl;
            fusebox.insert( std::pair<T*,fuse_ptr<T> * >(*ptr,this) );
        }
        
        // pointer access
        T* get() const
        {
            if( *ptr == 0 )
            {
                throw std::logic_error( "Usage of invalid CCharacter! Use isCharValid( char ) to check if a CCharacter is still valid." );
            }
            else
            {
                //std::cout << "Getting ptr of " << this << ": " << *ptr << std::endl;
                return *ptr; 
            }
        }
        
        operator T*() const
        {
            return get();
        }
        
        T* operator->() const
        {
            return get();
        }
        
        operator bool() const
        {
            return *ptr != 0;
        }
        
        // virtual destructor
        virtual ~fuse_ptr()
        {
            if( *ptr != 0 )
            {
                std::pair< typename std::multimap<T*,fuse_ptr<T> * >::iterator, typename std::multimap<T*,fuse_ptr<T> * >::iterator > range = fusebox.equal_range( *ptr );
                typename std::multimap<T*,fuse_ptr<T> * >::iterator it;
                for( it = range.first; it != range.second && it->second != this; ++it );
                if( it != range.second )
                {
                    //std::cout << this << " is not any longer fuse for " << it->first << std::endl;
                    fusebox.erase( it );
                }
            }
            delete ptr;
            ptr = 0;
        }

        // sets all associated fuse_ptrs' contents to null and removes them from fusebox
        static void blow_fuse( T* p )
        {
            std::pair< typename std::multimap<T*,fuse_ptr<T> * >::iterator, typename std::multimap<T*,fuse_ptr<T> * >::iterator > range = fusebox.equal_range( p );
            typename std::multimap<T*,fuse_ptr<T> * >::iterator it;
            for( it = range.first; it != range.second; ++it )
            {
                //std::cout << "Setting *ptr of " << it->second << " = 0" << std::endl;
                *(it->second->ptr) = 0;
            }
            fusebox.erase( p );
        }
};

template<class T>
std::multimap<T*,fuse_ptr<T> * > fuse_ptr<T>::fusebox;

// returns p.get()
template<class T>
T* get_pointer( fuse_ptr<T> const & p )
{
    return p.get();
}

template<class T>
bool isValid( fuse_ptr<T> const & p )
{
    return (bool)p;
}

#endif
