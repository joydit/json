// Copyright (c) 2015-2017 Dr. Colin Hirsch and Daniel Frey
// Please see LICENSE for license or visit https://github.com/taocpp/json/

#ifndef TAOCPP_JSON_INCLUDE_INTERNAL_GRAMMAR_HPP
#define TAOCPP_JSON_INCLUDE_INTERNAL_GRAMMAR_HPP

#include "../external/pegtl.hpp"
#include "../external/pegtl/contrib/abnf.hpp"

namespace tao
{
   namespace json
   {
      namespace internal
      {
         // clang-format off
         namespace rules
         {
            using namespace json_pegtl;

            struct ws : one< ' ', '\t', '\n', '\r' > {};

            template< typename R, typename P = ws >
            using padr = json_pegtl::internal::seq< R, json_pegtl::internal::star< P > >;

            struct begin_array : padr< one< '[' > > {};
            struct begin_object : padr< one< '{' > > {};
            struct end_array : one< ']' > {};
            struct end_object : one< '}' > {};
            struct name_separator : pad< one< ':' >, ws > {};
            struct value_separator : padr< one< ',' > > {};
            struct element_separator : padr< one< ',' > > {};

            struct false_ : TAOCPP_JSON_PEGTL_STRING( "false" ) {};
            struct null : TAOCPP_JSON_PEGTL_STRING( "null" ) {};
            struct true_ : TAOCPP_JSON_PEGTL_STRING( "true" ) {};

            struct digits : plus< abnf::DIGIT > {};

            struct zero : one< '0' > {};
            struct msign : one< '-' > {};
            struct esign : one< '-', '+' > {};

            struct edigits : digits {};
            struct fdigits : digits {};
            struct idigits : digits {};

            struct exp : seq< one< 'e', 'E' >, opt< esign >, must< edigits > > {};
            struct frac : if_must< one< '.' >, fdigits > {};
            struct int_ : sor< zero, idigits > {};
            struct number : seq< opt< msign >, int_, opt< frac >, opt< exp > > {};

            struct xdigit : abnf::HEXDIG {};
            struct unicode : list< seq< one< 'u' >, rep< 4, must< xdigit > > >, one< '\\' > > {};
            struct escaped_char : one< '"', '\\', '/', 'b', 'f', 'n', 'r', 't' > {};
            struct escaped : sor< escaped_char, unicode > {};

            struct unescaped
            {
               using analyze_t = analysis::generic< analysis::rule_type::ANY >;

               template< typename Input >
               static bool match( Input & in )
               {
                  bool result = false;

                  while ( ! in.empty() ) {
                     if ( const auto t = json_pegtl::internal::peek_utf8::peek( in ) ) {
                        if ( ( 0x20 <= t.data ) && ( t.data <= 0x10ffff ) && ( t.data != '\\' ) && ( t.data != '"' ) ) {
                           in.bump_in_this_line( t.size );
                           result = true;
                           continue;
                        }
                     }
                     return result;
                  }
                  throw std::logic_error( "code should be unreachable" );  // LCOV_EXCL_LINE
               }
            };

            struct chars : if_then_else< one< '\\' >, must< escaped >, unescaped > {};

            struct string_content : until< at< one< '"' > >, must< chars > > {};
            struct string : seq< one< '"' >, must< string_content >, any >
            {
               using content = string_content;
            };

            struct key_content : until< at< one< '"' > >, must< chars > > {};
            struct key : seq< one< '"' >, must< key_content >, any >
            {
               using content = key_content;
            };

            struct value;

            struct array_element;
            struct array_content : opt< list_must< array_element, element_separator > > {};
            struct array : seq< begin_array, array_content, must< end_array > >
            {
               using begin = begin_array;
               using end = end_array;
               using element = array_element;
               using content = array_content;
            };

            struct member : if_must< key, name_separator, value > {};
            struct object_content : opt< list_must< member, value_separator > > {};
            struct object : seq< begin_object, object_content, must< end_object > >
            {
               using begin = begin_object;
               using end = end_object;
               using element = member;
               using content = object_content;
            };

            struct sor_value
            {
               using analyze_t = analysis::generic< analysis::rule_type::SOR, string, number, object, array, false_, true_, null >;

               template< apply_mode A, rewind_mode M, template< typename ... > class Action, template< typename ... > class Control, typename Input, typename ... States >
               static bool match( Input & in, States && ... st )
               {
                  switch( in.peek_char() ) {
                     case '"': return Control< string >::template match< A, M, Action, Control >( in, st ... );
                     case '{': return Control< object >::template match< A, M, Action, Control >( in, st ... );
                     case '[': return Control< array >::template match< A, M, Action, Control >( in, st ... );
                     case 'n': return Control< null >::template match< A, M, Action, Control >( in, st ... );
                     case 't': return Control< true_ >::template match< A, M, Action, Control >( in, st ... );
                     case 'f': return Control< false_ >::template match< A, M, Action, Control >( in, st ... );
                     default: return Control< number >::template match< A, M, Action, Control >( in, st ... );
                  }
               }
            };

            struct value : padr< seq< sor_value, discard > > {};
            struct array_element : value {};

            struct text : seq< star< ws >, value > {};

         }  // namespace rules

         struct grammar : json_pegtl::must< rules::text, json_pegtl::eof > {};
         // clang-format on

      }  // namespace internal

   }  // namespace json

}  // namespace tao

#endif
