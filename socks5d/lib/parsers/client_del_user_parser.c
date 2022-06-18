//
// Created by tluci on 17/6/2022.
//

#include "parsers/client_del_user_parser.h"
#include "utils/logger.h"

ClientDelUserParserState traverseWordDel(ClientDelUserParser *p, byte c, ClientDelUserParserState nextState, char *nextWord) {
//    LogError(false, "char c = %c", c);
    if(strlen(p->Word) == p->Index){
        if(c == '|'){
            p->Word = nextWord;
            p->Index = 0;
            return nextState;
        }
        LogError(false, "Im in the last letter of the word and there is no pipe");
        return DelInvalidState;
    }
    if (c == p->Word[p->Index]) {
        p->Index++;
        return p->State;
    }
    LogError(false, "wrong character for word, was waiting for %c and got %c", p->Word[p->Index], c);
    return DelInvalidState;

}

void ClientDelUserParserReset(ClientDelUserParser *p) {
    LogInfo("Resetting DelParser...");
    if (null == p) {
        LogError(false, "Cannot reset NULL DelParser");
        return;
    }

    p->State = Del;
    p->Index = 0;

    memset(p->UName, 0, MAXLONG);
    p->Del[0] = 'D';
    p->Del[1] = 'E';
    p->Del[2] = 'L';
    p->Del[3] = 0;
    p->User[0] = 'U';
    p->User[1] = 'S';
    p->User[2] = 'E';
    p->User[3] = 'R';
    p->User[4] = 0;

    p->Word = p->Del;
    LogInfo("DelUserParser reset!");
}

ClientDelUserParserState ClientDelUserParserFeed(ClientDelUserParser *p, byte c) {
    LogInfo("Feeding %d to ClientDelUserParser", c);
//    LogError(false, "char= %c", c);
    if (null == p) {
        LogError(false, "Cannot feed DelUserParser if is NULL");
        return DelInvalidState;
    }

    switch (p->State) {
        case Del:
            p->State = traverseWordDel(p, c, DelUser, p->User);
            break;
        case DelUser:
            p->State = traverseWordDel(p, c, DelUserName, p->UName);
            break;
        case DelUserName:
            if(c == '|'){
                LogError(false, "Too many arguments");
                p->State = DelInvalidState;
                break;
            }
            if(c == '\r'){
                p->State = DelCRLF;
                break;
            }
            if(p->Index == MAXLONG+1){
                LogError(false, "Username can have max 255 characters");
                p->State = DelInvalidState;
                break;
            }
            p->Word[p->Index] = (char)c;
            p->Index++;
            break;
        case DelCRLF:
            if(c == '\n'){
                if(p->Index == 0){
                    p->State = DelInvalidState;
                    LogError(false, "Username has to be at least 1 character long");
                    break;
                }
                p->State = DelFinished;
                break;
            }
            if(c == '|'){
                LogError(false, "Too many arguments");
                p->State = DelInvalidState;
                break;
            }
            if(c == '\r'){
                if(p->Index == MAXLONG+1){
                    LogError(false, "Username can have max 255 characters");
                    p->State = DelInvalidState;
                    break;
                }
                p->Word[p->Index] = '\r';
                p->Index++;
                break;
            }

            if(p->Index == MAXLONG){
                LogError(false, "Username and password can have max 255 characters");
                p->State = DelInvalidState;
                break;
            }

            p->Word[p->Index] = '\r';
            p->Index++;
            p->Word[p->Index] = (char)c;
            p->Index++;
            p->State = DelUserName;
            break;
        case DelFinished:
        case DelInvalidState:
            break;
    }
    return p->State;
}

bool ClientDelUserParserHasFailed(ClientDelUserParserState state) {
    return state == DelInvalidState ? true : false;
}

size_t ClientDelUserParserConsume(ClientDelUserParser *p, byte *c, size_t length) {
    LogInfo("DelUserParser consuming %d bytes", length);
    if (null == p) {
        LogError(false, "Cannot consume if DelUserParser is NULL");
        return 0;
    }

    if (null == c) {
        LogError(false, "DelUserParser cannot consume NULL array");
        return 0;
    }

    for (size_t i = 0; i < length; ++i) {
        ClientDelUserParserState state = ClientDelUserParserFeed(p, c[i]);
        if (ClientDelUserParserHasFinished(state))
            return i + 1;
    }
    return length;
}

bool ClientDelUserParserHasFinished(ClientDelUserParserState state) {
    switch (state) {
        default:
        case Del:
        case DelUser:
        case DelUserName:
        case DelCRLF:
            return false;
        case DelInvalidState:
        case DelFinished:
            return true;
    }
}
