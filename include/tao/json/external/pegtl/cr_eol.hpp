// Copyright (c) 2016-2017 Dr. Colin Hirsch and Daniel Frey
// Please see LICENSE for license or visit https://github.com/taocpp/PEGTL/

#ifndef TAOCPP_JSON_PEGTL_INCLUDE_CR_EOL_HPP
#define TAOCPP_JSON_PEGTL_INCLUDE_CR_EOL_HPP

#include "config.hpp"

namespace tao
{
   namespace TAOCPP_JSON_PEGTL_NAMESPACE
   {
      struct cr_eol
      {
         static constexpr int ch = '\r';

         template< typename Input >
         static eol_pair match( Input& in )
         {
            eol_pair p = { false, in.size( 1 ) };
            if( p.second ) {
               if( in.peek_char() == '\r' ) {
                  in.bump_to_next_line();
                  p.first = true;
               }
            }
            return p;
         }
      };

   }  // namespace TAOCPP_JSON_PEGTL_NAMESPACE

}  // namespace tao

#endif
