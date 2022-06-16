//
// Created by tluci on 14/6/2022.
//

#include "client_hello_parser_test.h"
#include "parsers/client_hello_parser.h"
ClientHelloParser parser;

/**************************************************************************
 *                      ClientHelloParserReset Tests
 ***************************************************************************/
START_TEST(Reset_Succeeds)
    {

        // Arrange
        // Act
        ClientHelloParserReset(&parser);
        // Assert
        for (int i = 0; i < 51; ++i) {
            ck_assert_int_eq(parser.Passwd[i], 0);
            ck_assert_int_eq(parser.UName[i], 0);
        }
        ck_assert_int_eq(parser.word[0], 'H');
        ck_assert_int_eq(parser.word[1], 'E');
        ck_assert_int_eq(parser.word[2], 'L');
        ck_assert_int_eq(parser.word[3], 'L');
        ck_assert_int_eq(parser.word[4], 'O');
        ck_assert_int_eq(parser.State, Hello);
        ck_assert_int_eq(parser.index, 0);
    }
END_TEST
/**************************************************************************
 *                      AuthParserFeed Tests
 ***************************************************************************/

START_TEST(Feed_NullParser_Fails)
    {
        // Arrange
        ClientHelloParser *nullParser = null;
        // Act
        ClientHelloParserState state = ClientHelloParserFeed(nullParser, 0);
        // Assert
        ck_assert_int_eq(state, HelloInvalidState);
    }
END_TEST

START_TEST(Feed_HELLO_CR_COMPLETE_Succeeds)
    {

        // Arrange
        ClientHelloParserReset(&parser);
        byte message[] = {'H', 'E', 'L','L', 'O', '|', 'u', 's', 'e','r','\r', '1', '|', 'p', 'a', 's', 's', '\r', '\n'};
        ClientHelloParserState state;
        // Act
        for (int i=0; i < (int)sizeof(message)/(int) sizeof(message[0]) && state != HelloInvalidState; i++ ){
            state = ClientHelloParserFeed(&parser, message[i]);
        }

        // Assert
        ck_assert_int_eq(state, HelloFinished);
    }
END_TEST

START_TEST(Feed_HELLO_CRLF_Username_COMPLETE_Fails)
    {

        // Arrange
        ClientHelloParserReset(&parser);
        byte message[] = {'H', 'E', 'L','L', 'O', '|', 'u', 's', 'e','r','\r','\n', '1', '|', 'p', 'a', 's', 's', '\r', '\n'};
        ClientHelloParserState state;
        // Act
        for (int i=0; i < (int)sizeof(message)/(int) sizeof(message[0]) && state != HelloInvalidState; i++ ){
            state = ClientHelloParserFeed(&parser, message[i]);
        }

        // Assert
        ck_assert_int_eq(state, HelloInvalidState);
    }
END_TEST

START_TEST(Feed_HELLO_COMPLETE_Succeeds)
    {

        // Arrange
        ClientHelloParserReset(&parser);
        byte message[] = {'H', 'E', 'L','L', 'O', '|', 'u', 's', 'e','r', '1', '|', 'p', 'a', 's', 's', '\r', '\n'};
        ClientHelloParserState state;
        // Act
        for (int i=0; i < (int)sizeof(message)/(int) sizeof(message[0]) && state != HelloInvalidState; i++ ){
            state = ClientHelloParserFeed(&parser, message[i]);
        }

        // Assert
        ck_assert_int_eq(state, HelloFinished);
    }
END_TEST
START_TEST(Feed_HELLOCRLF_Fails)
    {

        // Arrange
        ClientHelloParserReset(&parser);
        byte message[] = {'H', 'E', 'L','L', 'O', '|', 'u', 's', 'e','r', '1', '|', 'p', 'a', 's', 's'};
        ClientHelloParserState state;
        // Act
        for (int i=0; i < (int)sizeof(message)/(int) sizeof(message[0]) && state != HelloInvalidState; i++ ){
            state = ClientHelloParserFeed(&parser, message[i]);
        }

        // Assert
        ck_assert_int_eq(state, HelloPassword);
    }
END_TEST

START_TEST(Feed_HELLO_Succeeds)
    {

        // Arrange
        ClientHelloParserReset(&parser);

        byte message[] = {'H', 'E', 'L','L', 'O', '|'};
        ClientHelloParserState state;
        // Act
        for (int i=0; i < (int)sizeof(message)/(int) sizeof(message[0]) && state != HelloInvalidState; i++ ){
            state = ClientHelloParserFeed(&parser, message[i]);
        }

        // Assert
        ck_assert_int_eq(state, HelloUsername);
    }
END_TEST

START_TEST(Feed_HELLOUsername_Succeeds)
    {

        // Arrange
        ClientHelloParserReset(&parser);

        byte message[] = {'H', 'E', 'L','L', 'O', '|', 'u', 's', 'e','r', '1', '|'};
        ClientHelloParserState state;
        // Act
        for (int i=0; i < (int)sizeof(message)/(int) sizeof(message[0]) && state != HelloInvalidState; i++ ){
            state = ClientHelloParserFeed(&parser, message[i]);
        }

        // Assert
        ck_assert_int_eq(state, HelloPassword);
    }
END_TEST




Suite *RegisterClientHelloParserTestSuit() {
    Suite *s = suite_create("HelloClientParser");

    TCase *tc = tcase_create("HelloClientParser");
    tcase_add_test(tc, Reset_Succeeds);
    tcase_add_test(tc, Feed_NullParser_Fails);
    tcase_add_test(tc, Feed_HELLO_COMPLETE_Succeeds);
    tcase_add_test(tc, Feed_HELLOCRLF_Fails);
    tcase_add_test(tc, Feed_HELLOUsername_Succeeds);
    tcase_add_test(tc, Feed_HELLO_Succeeds);
    tcase_add_test(tc, Feed_HELLO_CR_COMPLETE_Succeeds);
    tcase_add_test(tc, Feed_HELLO_CRLF_COMPLETE_Fails);
    suite_add_tcase(s, tc);

    return s;
}

