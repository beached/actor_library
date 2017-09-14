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
#include <map>
#include <string>

#include <daw/daw_exception.h>
#include <daw/fs/message_queue.h>

namespace daw {
	struct actor_dispatch_t {
		virtual ~actor_dispatch_t( ) = 0;
		virtual std::string get_type_name( ) const = 0;
		virtual void receive_message( boost::any ) = 0;
	};

	struct director_t {
		using director_cb_t = std::function<void(boost::any)>;
		std::map<size_t, director_cb_t> m_actors;
		std::map<std::string, size_t> m_actor_map;	
	};

	struct invalid_message_type: std::runtime_error { };

	bool any_true( std::initializer_list<bool> tests ) {
		for( bool test: tests ) {
			if( test ) {
				return true;
			}
		}
		return false;
	}

	template<typename Acceptor, typename DispatchTag, typename... Accepts>
	class actor_t: public actor_dispatch_t {
		Acceptor m_acceptor;
	  public:
		actor_t( Acceptor acceptor ) : m_acceptor{std::move( acceptor )} {}
		virtual ~actor_t( ) = default;
		virtual std::string get_type_name( ) const = 0;
		actor_t( actor_t const & ) = default;
		actor_t( actor_t && ) noexcept = default;
		actor_t & operator=( actor_t const & ) = default;
		actor_t & operator=( actor_t && ) noexcept = default;

		void receive_message( boost::any message ) {
			daw::exception::daw_throw_on_false<invalid_message_type>(
			    any_true( {message.type( ) == typeid( Accepts )...} ), "Message is not of a valid type" );

			m_acceptor( std::move( message ) );
		}
	};

} // namespace daw

