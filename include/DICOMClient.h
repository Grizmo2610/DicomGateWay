#ifndef DICOMCLIENT_H
#define DICOMCLIENT_H

#include "dcmtk/dcmnet/assoc.h"
#include "dcmtk/dcmnet/dul.h"
#include "dcmtk/dcmnet/dimse.h"
#include "dcmtk/dcmdata/dcuid.h"
#include <iostream>
#include <memory>
#include <vector>

using namespace std;

class DICOMClient {
    T_ASC_Network *net = nullptr;
    T_ASC_Association *assoc = nullptr;
    T_ASC_Parameters *params = nullptr;
    string aeTitleSCU;
    string aeTitleSCP;
    string peerAddress;
    int peerPort;
    string dicomDictPath;
    [[nodiscard]] bool sendCEcho(int msgId) const;
    [[nodiscard]] bool sendCStore(int msgId, const string &dicomFilePath) const;
    [[nodiscard]] bool sendCFind(int msgId,
                                DcmDataset &query,
                                vector<string> &foundFiles,
                                int &responseCount) const;

public:
    explicit DICOMClient(
        string aeSCU = "SCU_AET",
        string aeSCP = "PACS_SRC",
        string address = "127.0.0.1",
        int port = 1024,
        string dicomDict = "/mnt/c/usr/local/share/dicom.dic"
        );
    ~DICOMClient();

    bool connect(const char *abstractSyntax, const char *transferSyntax, T_ASC_PresentationContextID presentationContextID = 1);
    [[nodiscard]] bool sendMessage(
                int msgId,
                const string &dicomFilePath = "",
                DcmDataset query = DcmDataset(),
                vector<string> *foundFiles = {},
                int count = 0) const;

    static DcmDataset createFindQuery(const string &patientName = "",
                                      const string &patientID = "",
                                      const string &studyDate = "",
                                      const string &modality = "",
                                      const string &accessionNumber = "");

    void disconnect();
};

#endif