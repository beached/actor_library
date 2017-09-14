// The MIT License (MIT)
//
// Copyright (c) 2017 Darrell Wright
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files( the "Software" ), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and / or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#pragma once

#include <boost/any.hpp>
#include <functional>
#include <string>
#include <typeindex>
#include <unordered_map>

#include <daw/daw_exception.h>
#include <daw/fs/message_queue.h>

#include "helper.h"

namespace daw {
	struct actor_dispatch_t {
		virtual ~actor_dispatch_t( ) = 0;
		virtual std::string get_type_name( ) const = 0;
		virtual void receive_message( boost::any ) = 0;
	};

	struct director_t {
		using director_cb_t = std::function<void( boost::any )>;
		std::map<size_t, director_cb_t> m_actors;
		std::map<std::string, size_t> m_actor_map;
	};

	struct invalid_message_type : std::runtime_error {};

	template<typename DispatchTag>
	class actor_t : public actor_dispatch_t {
		using receiver_function_t = std::function<void( boost::any )>;
		std::unordered_map<std::type_index, receiver_function_t> m_receivers;

	  protected:
		struct any_message_tag {};
		template<typename T>
		void add_receiver( std::function<void( T )> receiver_func ) {
			m_receivers.insert( std::make_pair(
			    std::type_index{typeid( T )}, [receiver_func = std::move( receiver_func )]( boost::any msg ) {
				    receiver_func( boost::any_cast<T>( msg ) );
			    } ) );
		}

		void add_receiver( std::function<void( boost::any )> receiver_func ) {
			m_receivers.insert(
			    std::make_pair( std::type_index{typeid( any_message_tag )}, std::move( receiver_func ) ) );
		}

	  public:
		actor_t( ) = default;
		virtual ~actor_t( ) = default;
		virtual std::string get_type_name( ) const = 0;
		actor_t( actor_t const & ) = default;
		actor_t( actor_t && ) noexcept = default;
		actor_t &operator=( actor_t const & ) = default;
		actor_t &operator=( actor_t && ) noexcept = default;

		void receive_message( boost::any const &msg ) {
			auto it = m_receivers.find( std::type_index{msg.type( )} );
			if( it == m_receivers.cend( ) ) {
				it = m_receivers.find( std::type_index{typeid( any_message_tag )} );
				if( it != m_receivers.cend( ) ) {
					it->second( msg );
				} else {
					// Invalid message type
				}
				return;
			}
			try {
				it->second( msg );
			} catch( ... ) {
				// TODO: add error handling for exception in receiver
				return;
			}
		}
	};
} // namespace daw

