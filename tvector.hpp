// thread safe list to pass objects from one thread to another...
// only use push_back, pop_front or clear!
// note: pop_front returns the object as well as deleting it from the list...

#ifndef __tvector_hpp
#define __tvector_hpp

#include <list>
#include <exception>
#include <iostream>
#include <cerrno> // error numbers.. EAGAIN etc.
#include <pthread.h>

#ifndef _MAX_NON_BLOCK_TRYS2_
#define _MAX_NON_BLOCK_TRYS2_ 20
#endif

template<class T> class tvector : public std::list<T> {
	public:
		tvector() : std::list<T>() {
			// initialize our mutex
			vlock = new pthread_mutex_t;

			if (pthread_mutex_init(vlock,NULL)) {
				std::cout << "Mutex couldn't get initialized... throwing exception!" << std::endl;
				throw std::exception();
			}
		}

		~tvector() {
			pthread_mutex_destroy(vlock);
			delete vlock;
		}

		inline void clear() {
			// get mutex
			if (pthread_mutex_lock(vlock)) {
				std::cout << "problem with mutex locking!" << std::endl;
			}

			std::list<T>::clear();

			// release mutex
			if (pthread_mutex_unlock(vlock)) {
				std::cout << "problem with mutex unlocking occured!" << std::endl;
			}
		}

		inline void push_back(const T& item) {
			// get mutex
			if (pthread_mutex_lock(vlock)) {
				std::cout << "problem with mutex locking!" << std::endl;
			}

			std::list<T>::push_back(item);

			// release mutex
			if (pthread_mutex_unlock(vlock)) {
				std::cout << "problem with mutex unlocking occured!" << std::endl;
			}
		}
        
        inline bool non_block_push_back( const T &item)
        {
            for ( int i = 0; i < _MAX_NON_BLOCK_TRYS2_ ; ++i)
            {
                int err = pthread_mutex_trylock(vlock);
                if ( err )
                {
                    timespec stime;
                    stime.tv_sec = 0;
                    stime.tv_nsec = 5000000;
                    std::cout<<"non_block_push_back blocked mutex"<<std::endl;
                    nanosleep( &stime, NULL);
                }
                else
                {
                    std::list<T>::push_back(item);
                    pthread_mutex_unlock(vlock);
                    return true;
                }
            } 
            return false;
        }        
        
        inline bool empty()
        {
			if (pthread_mutex_lock(vlock)) 
            {
				std::cout << "problem with mutex locking!" << std::endl;
			}

			bool empty = std::list<T>::empty();
			// release mutex
			if (pthread_mutex_unlock(vlock)) 
            {
				std::cout << "problem with mutex unlocking occured!" << std::endl;
			}

			return empty;        
        }

		inline T& pop_front() {
			// get mutex
			if (pthread_mutex_lock(vlock)) {
				std::cout << "problem with mutex locking!" << std::endl;
			}

			T& item = std::list<T>::front();
			std::list<T>::pop_front();

			// release mutex
			if (pthread_mutex_unlock(vlock)) {
				std::cout << "problem with mutex unlocking occured!" << std::endl;
			}

			return item;
		}
        
        inline T non_block_pop_front()
        {
            for ( int i = 0; i < _MAX_NON_BLOCK_TRYS2_ ; ++i)
            {
                int err = pthread_mutex_trylock(vlock);
                if ( err )
                {
                    timespec stime;
                    stime.tv_sec = 0;
                    stime.tv_nsec = 5000000;
                    std::cout<<"non_block_push_back blocked mutex"<<std::endl;
                    nanosleep( &stime, NULL);
                }
                else
                {
                    T item = std::list<T>::front();
           			std::list<T>::pop_front();
                    pthread_mutex_unlock(vlock);
                    return item;
                }
            }
            return T();            
        }        

	private:
		pthread_mutex_t *vlock;

};

#endif
