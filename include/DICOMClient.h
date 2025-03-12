#ifndef DICOMCLIENT_H
#define DICOMCLIENT_H

#include "dcmtk/dcmnet/assoc.h"
#include "dcmtk/dcmnet/dul.h"
#include "dcmtk/dcmnet/dimse.h"
#include "dcmtk/dcmdata/dcuid.h"
#include <iostream>

using namespace std;

class DICOMClient {
private:
    T_ASC_Network *net = nullptr;
    T_ASC_Association *assoc = nullptr;
    T_ASC_Parameters *params = nullptr;
    string aeTitleSCU;
    string aeTitleSCP;
    string peerAddress;
    int peerPort;
    string dicomDictPath;

public:
    explicit DICOMClient(
        string aeSCU = "SCU_AET",
        string aeSCP = "SCP_AET",
        string address = "127.0.0.1",
        int port = 1024,
        string dicomDict = "/mnt/c/usr/local/share/dicom.dic"
        );
    ~DICOMClient();

    bool connect(const char *abstractSyntax, const char *transferSyntax, T_ASC_PresentationContextID presentationContextID = 1);
    bool sendMessage(int msgId, const string &dicomFilePath = nullptr);

    bool sendCEcho(int msgId);
    bool sendCStore(int msgId, const string &dicomFilePath);
    bool sendCFind(int msgId);
    void disconnect();
};


#endif