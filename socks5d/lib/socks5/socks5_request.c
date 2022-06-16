//
// Created by Lucas Dell'Isola on 14/06/2022.
//

#include <memory.h>
#include <pthread.h>
#include <stdio.h>
#include "socks5/socks5_request.h"
#include "socks5/socks5_connection.h"
#include "utils/logger.h"
#include "socks5/socks5_messages.h"

void * ResolveDNS(void *data);

#define ATTACHMENT(key) ( (Socks5Connection*)((SelectorKey*)(key))->Data)

void RequestReadInit(unsigned int state, void *data) {

    Socks5Connection *connection = ATTACHMENT(data);
    RequestData *d = &connection->Data.Request;
    d->Command = SOCKS5_REPLY_NOT_DECIDED;
    d->ReadBuffer = &connection->ReadBuffer;
    d->WriteBuffer = &connection->WriteBuffer;
    RequestParserReset(&d->Parser);
}

void RequestReadClose(unsigned int state, void *data) {

}


unsigned RequestReadRun(void *data) {
    Socks5Connection *connection = ATTACHMENT(data);
    RequestData *d = &connection->Data.Request;
    size_t bufferSize;
    byte *buffer = BufferWritePtr(d->ReadBuffer, &bufferSize);
    ssize_t bytesRead = ReadFromTcpConnection(connection->ClientTcpConnection, buffer, bufferSize);

    if (bytesRead < 0) {
        LogError(false, "Cannot read from Tcp connection");
        return CS_ERROR;
    }

    BufferWriteAdv(d->ReadBuffer, bytesRead);
    RequestParserConsume(&d->Parser, buffer, bytesRead);

    if (!RequestParserHasFinished(d->Parser.State))
        return CS_REQUEST_READ;

    if (RequestParserFailed(d->Parser.State))
        return CS_ERROR;

    if (d->Parser.CMD != SOCKS5_CMD_CONNECT) {
        d->Command = SOCKS5_REPLY_COMMAND_NOT_SUPPORTED;
        return SELECTOR_STATUS_SUCCESS == SelectorSetInterestKey(data, SELECTOR_OP_WRITE) ? CS_REQUEST_WRITE : CS_ERROR;
    }

    if (d->Parser.AType == SOCKS5_ADDRESS_TYPE_FQDN) {

        SelectorKey * allocatedKey = malloc(sizeof(SelectorKey));
        if(null == allocatedKey)
            d->Command = SOCKS5_REPLY_GENERAL_FAILURE;
        else
        {
//            memcpy(allocatedKey, data, sizeof(SelectorKey));
//            if(-1 == pthread_create(&tid, 0, ResolveDNS, allocatedKey))
//                d->Command = SOCKS5_REPLY_GENERAL_FAILURE;
//            else
//                return SELECTOR_STATUS_SUCCESS == SelectorSetInterestKey(data, SELECTOR_OP_NOOP) ? CS_DNS_READ : CS_ERROR;
//
        }
        return SELECTOR_STATUS_SUCCESS == SelectorSetInterestKey(data, SELECTOR_OP_WRITE) ? CS_REQUEST_WRITE : CS_ERROR;
    }

//    d->UsesDns = false;
    d->RemoteAddress = null;
    d->CurrentRemoteAddress = calloc(1,sizeof(struct addrinfo));
    d->CurrentRemoteAddress->ai_socktype = SOCK_STREAM;
    d->CurrentRemoteAddress->ai_flags = AI_PASSIVE;

    if (SOCKS5_ADDRESS_TYPE_IPV4 == d->Parser.AType){
        struct sockaddr_in * in = calloc(1, sizeof(struct sockaddr_in));
        d->CurrentRemoteAddress->ai_family = AF_INET;
        in->sin_family = AF_INET;
        memcpy(&in->sin_addr,d->Parser.DestAddress,4);
        memcpy(&in->sin_port,d->Parser.DestPort,2);

        d->CurrentRemoteAddress->ai_addr = (struct sockaddr *) in;
    }

    if (SOCKS5_ADDRESS_TYPE_IPV6 == d->Parser.AType){
        struct sockaddr_in6 * in6 = calloc(1, sizeof(struct sockaddr_in6));
        d->CurrentRemoteAddress->ai_family = AF_INET6;
        in6->sin6_family = AF_INET6;
        memcpy(&in6->sin6_addr,d->Parser.DestAddress,16);
        memcpy(&in6->sin6_port,d->Parser.DestPort,2);

        d->CurrentRemoteAddress->ai_addr = (struct sockaddr *) in6;
    }


    return SELECTOR_STATUS_SUCCESS == SelectorSetInterestKey(data, SELECTOR_OP_NOOP) ? CS_ESTABLISH_CONNECTION
                                                                                     : CS_ERROR;
}

void RequestWriteClose(unsigned int state, void *data) {
    Socks5Connection *connection = ATTACHMENT(data);
    RequestData *d = &connection->Data.Request;

    BufferReset(d->WriteBuffer);
    BufferReset(d->ReadBuffer);
}

void RequestWriteInit(unsigned int state, void *data) {
    Socks5Connection *connection = ATTACHMENT(data);
    RequestData *d = &connection->Data.Request;

    d->WriteBuffer = &connection->WriteBuffer;

    size_t size;
    byte *buffer = BufferWritePtr(d->WriteBuffer, &size);
    size_t messageSize = BuildRequestResponseFromParser(buffer, size, d->Command, &d->Parser);
    BufferWriteAdv(d->WriteBuffer, messageSize);

}

unsigned RequestWriteRun(void *data) {
    Socks5Connection *connection = ATTACHMENT(data);
    RequestData *d = &connection->Data.Request;
    fd_selector selector = ((SelectorKey *) data)->Selector;

    if (!BufferCanRead(d->WriteBuffer)) {

        if (SOCKS5_REPLY_SUCCEEDED != d->Command){
            return CS_DONE;
        }

        if (
                SELECTOR_STATUS_SUCCESS ==
                SelectorSetInterest(selector, connection->ClientTcpConnection->FileDescriptor, SELECTOR_OP_READ) &&
                SELECTOR_STATUS_SUCCESS ==
                SelectorSetInterest(selector, connection->RemoteTcpConnection->FileDescriptor, SELECTOR_OP_READ)
                ) {
            return CS_CONNECTED;
        }
        return CS_ERROR;
    }

    size_t size;
    byte *ptr = BufferReadPtr(d->WriteBuffer, &size);

    size_t bytesWritten = WriteToTcpConnection(connection->ClientTcpConnection, ptr, size);
    BufferReadAdv(d->WriteBuffer, bytesWritten);

    return CS_REQUEST_WRITE;
}

void * ResolveDNS(void *data){
    SelectorKey * key = (SelectorKey*) data;
    Socks5Connection * connection = ATTACHMENT(data);
    RequestData * d = &connection->Data.Request;

    pthread_detach(pthread_self());

    d->RemoteAddress = null;
    struct addrinfo hints = {
            .ai_family = AF_UNSPEC,
            .ai_socktype = SOCK_STREAM,
            .ai_flags = AI_PASSIVE,
            .ai_protocol = 0,
            .ai_canonname = null,
            .ai_addr = null,
            .ai_next = null
    };

    char buff[10];
    snprintf(buff, sizeof(buff),"%d", ntohs(GetPortNumberFromNetworkOrder(d->Parser.DestPort)));

    int result = getaddrinfo(
            (const char *)d->Parser.DestAddress,
                buff,
                &hints,
                &d->RemoteAddress
    );

    if (0 != result)
    {
        d->RemoteAddress = null;
    }

    SelectorNotifyBlock(key->Selector,key->Fd);
    free(data);

    return 0;
}














