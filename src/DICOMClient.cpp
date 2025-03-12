#include "DICOMClient.h"
#include "dcmtk/dcmdata/dctk.h"

#include <utility>

using namespace std;
DICOMClient::DICOMClient(   string aeSCU,
                            string aeSCP,
                            string address,
                            int port,
                            string dicomDict)
    :
        aeTitleSCU(move(aeSCU)),
        aeTitleSCP(move(aeSCP)),
        peerAddress(move(address)),
        peerPort(port),
        dicomDictPath(move(dicomDict)){

    if (setenv("DCMDICTPATH", dicomDictPath.c_str(), 1)) {
        cerr << "setenv(DCMDICTPATH) failed" << endl;
    }else {
        cout << "DICOM dictionary path set." << endl;
    }

}


DICOMClient::~DICOMClient() {
    // disconnect();
}
bool DICOMClient::connect(const char *abstractSyntax, const char *transferSyntax, T_ASC_PresentationContextID presentationContextID){
    if (ASC_initializeNetwork(NET_REQUESTOR, peerPort, 1000, &net).bad()) {
        cerr << "Failed to initialize DICOM network." << endl;
        return false;
    }

    if (ASC_createAssociationParameters(&params, 16372, 30).bad()) {
        cerr << "Failed to create association parameters." << endl;
        return false;
    }

    if (const OFCondition status = ASC_addPresentationContext(params, presentationContextID, abstractSyntax, &transferSyntax, 1); status.bad()) {
        cerr << "Failed to add presentation context." << endl;
        return false;
    }
    cout << "DICOM add presentation context." << endl;

    ASC_setAPTitles(params, aeTitleSCU.c_str(), aeTitleSCP.c_str(), nullptr);
    string peer = peerAddress + ":" + to_string(peerPort);
    ASC_setPresentationAddresses(params, peer.c_str(), peer.c_str());

    if (ASC_requestAssociation(net, params, &assoc).bad()) {
        cerr << "Failed to request association." << endl;
        return false;
    }

    if (ASC_countAcceptedPresentationContexts(params) == 0) {
        cerr << "No acceptable presentation contexts!" << endl;
        return false;
    }

    cout << "DICOM Association established successfully!" << endl;
    return true;
}

bool DICOMClient::sendMessage(const int msgId, const string &dicomFilePath) {
    if (!assoc) {
        cerr << "No active association!" << endl;
        return false;
    }
    switch (msgId) {
        case 1: // C-ECHO
            return sendCEcho(msgId);
        case 2: // C-FIND
            return sendCFind(msgId);
        case 3: // C-STORE
            return sendCStore(msgId, dicomFilePath);
        default:
            cerr << "Unknown message ID: " << msgId << endl;
        return false;
    }
}

bool DICOMClient::sendCEcho(const int msgId) {
    DIC_US rspStatus;
    DcmDataset *statusDetail = nullptr;

    if (const OFCondition status = DIMSE_echoUser(assoc, msgId, DIMSE_BLOCKING, 10, &rspStatus, &statusDetail);
        status.good() && rspStatus == STATUS_Success) {
        cout << "C-ECHO successful! (msgId: " << msgId << ")" << endl;
        return true;
    } else {
        cerr << "C-ECHO failed: " << status.text() << " (Status: " << rspStatus << ", msgId: " << msgId << ")" << endl;
        return false;
    }
}

bool DICOMClient::sendCStore(int msgId, const string &dicomFilePath) {
    if (!assoc) {
        cerr << "No active association!" << endl;
        return false;
    }

    if (dicomFilePath.empty()) {
        cerr << "C-STORE requires a valid DICOM file path!" << endl;
        return false;
    }

    DcmFileFormat dcmFile;
    OFCondition status = dcmFile.loadFile(dicomFilePath.c_str());

    if (status.bad()) {
        cerr << "Failed to load DICOM file: " << dicomFilePath << endl;
        return false;
    }

    DcmDataset *dataset = dcmFile.getDataset();
    if (!dataset) {
        cerr << "DICOM file is empty or invalid." << endl;
        return false;
    }

    T_DIMSE_C_StoreRQ request;

    OFString sopInstanceUID, sopClassUID;
    if (dataset->findAndGetOFString(DCM_SOPInstanceUID, sopInstanceUID).bad()) {
        cerr << "Failed to get SOPInstanceUID from dataset!" << endl;
        return false;
    }
    if (dataset->findAndGetOFString(DCM_SOPClassUID, sopClassUID).bad()) {
        cerr << "Failed to get SOPClassUID from dataset!" << endl;
        return false;
    }

    memset(&request, 0, sizeof(request));

    strcpy(request.AffectedSOPInstanceUID, sopInstanceUID.c_str());
    strcpy(request.AffectedSOPClassUID, sopClassUID.c_str());

    request.MessageID = msgId;
    request.DataSetType = DIMSE_DATASET_PRESENT;

    if (dataset->findAndGetOFString(DCM_SOPInstanceUID, sopInstanceUID).bad()) {
        cerr << "Failed to get SOPInstanceUID from dataset!" << endl;
        return false;
    }
    strcpy(request.AffectedSOPInstanceUID, sopInstanceUID.c_str());


    T_ASC_PresentationContextID presId = ASC_findAcceptedPresentationContextID(assoc, request.AffectedSOPClassUID);
    if (presId == 0) {
        cerr << "No suitable presentation context for C-STORE!" << endl;
        return false;
    }

    T_DIMSE_C_StoreRSP response;
    DcmDataset *statusDetail = nullptr;
    status = DIMSE_storeUser(assoc, presId, &request, dicomFilePath.c_str(), dataset, nullptr, nullptr, DIMSE_BLOCKING, 0, &response, &statusDetail, nullptr, 0);

    if (status.good() && response.DimseStatus == STATUS_Success) {
        cout << "C-STORE successful! (msgId: " << msgId << ", File: " << dicomFilePath << ")" << endl;
        return true;
    }
    cerr << "C-STORE failed: " << status.text() << " (Status: " << response.DimseStatus << ", msgId: " << msgId << ")" << endl;
    return false;
}


bool DICOMClient::sendCFind(int msgId) {
    return false;
}


void DICOMClient::disconnect() {
    if (assoc) {
        if (ASC_releaseAssociation(assoc).bad()) {
            cerr << "Warning: Failed to release association, aborting..." << endl;
            ASC_abortAssociation(assoc);
        }
        ASC_destroyAssociation(&assoc);
    }

    if (net) {
        ASC_dropNetwork(&net);
    }

    cout << "DICOM Network shut down." << endl;
}
