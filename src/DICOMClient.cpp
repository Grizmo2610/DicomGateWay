#include "DICOMClient.h"
#include "dcmtk/dcmdata/dctk.h"
#include <vector>
#include <utility>

using namespace std;

DICOMClient::DICOMClient(string aeSCU, string aeSCP, string address, const int port, string dicomDict)
    : aeTitleSCU(move(aeSCU)),
      aeTitleSCP(move(aeSCP)),
      peerAddress(move(address)),
      peerPort(port),
      dicomDictPath(move(dicomDict)) {

    if (setenv("DCMDICTPATH", dicomDictPath.c_str(), 1) != 0) {
        cerr << "setenv(DCMDICTPATH) failed!" << endl;
    } else {
        cout << "DICOM dictionary path set: " << dicomDictPath << endl;
    }

    if (!getenv("DCMDICTPATH")) {
        cerr << "Warning: DCMDICTPATH is not set correctly!" << endl;
    }
}

void DICOMClient::disconnect() {
    if (assoc) {
        if (ASC_releaseAssociation(assoc).bad()) {
            cerr << "Warning: Failed to release association, aborting..." << endl;
            ASC_abortAssociation(assoc);
        }
        ASC_destroyAssociation(&assoc);
        assoc = nullptr;
    }

    if (net) {
        ASC_dropNetwork(&net);
        net = nullptr;
    }

    cout << "DICOM Network shut down." << endl;
}


DICOMClient::~DICOMClient() {
    disconnect();
}

bool DICOMClient::connect(const char *abstractSyntax, const char *transferSyntax, T_ASC_PresentationContextID presentationContextID) {
    if (ASC_initializeNetwork(NET_REQUESTOR, peerPort, 1000, &net).bad()) {
        cerr << "Failed to initialize DICOM network." << endl;
        disconnect();
        return false;
    }

    if (ASC_createAssociationParameters(&params, 16372, 30).bad()) {
        cerr << "Failed to create association parameters." << endl;
        disconnect();
        return false;
    }

    if (ASC_addPresentationContext(params, presentationContextID, abstractSyntax, &transferSyntax, 1).bad()) {
        cerr << "Failed to add presentation context." << endl;
        disconnect();
        return false;
    }

    cout << "DICOM add presentation context." << endl;

    if (const OFCondition status = ASC_setAPTitles(params, aeTitleSCU.c_str(), aeTitleSCP.c_str(), nullptr); status.bad()) {
        disconnect();
        cerr << "Failed to set AP Titles: " << status.text() << endl;
        return false;
    }
    const string peer = peerAddress + ":" + to_string(peerPort);

    if (const OFCondition status = ASC_setPresentationAddresses(params, peer.c_str(), peer.c_str()); status.bad()) {
        disconnect();
        cerr << "Failed to set Presentation ddresses: " << status.text() << endl;
        return false;
    }

    if (const OFCondition status = ASC_requestAssociation(net, params, &assoc); status.bad()) {
        cerr << "Failed to request association: " << status.text() << endl;
        disconnect();
        return false;
    }

    int acceptedCount = ASC_countAcceptedPresentationContexts(params);
    cout << "Accepted Presentation Contexts: " << acceptedCount << endl;

    if (ASC_countAcceptedPresentationContexts(params) == 0) {
        cerr << "No acceptable presentation contexts!" << endl;
        disconnect();
        return false;
    }

    cout << "DICOM Association established successfully!" << endl;
    return true;
}

bool DICOMClient::sendMessage(const int msgId,
                              const string &dicomFilePath,
                              DcmDataset query,
                              vector<string> *foundFiles,
                              int count) const {
    if (!assoc) {
        cerr << "No active association!" << endl;
        return false;
    }
    switch (msgId) {
        case 1: {
            return sendCEcho(msgId);
        }
        case 2: {
            return sendCFind(msgId, query, *foundFiles, count);
        }
        case 3: {
            return sendCStore(msgId, dicomFilePath);
        }
        default: {
            cerr << "Unknown message ID: " << msgId << endl;
            return false;
        }
    }
}

bool DICOMClient::sendCEcho(const int msgId) const {
    DIC_US rspStatus;
    DcmDataset *statusDetail = nullptr;
    const OFCondition status = DIMSE_echoUser(assoc, msgId, DIMSE_BLOCKING,
                                              10, &rspStatus, &statusDetail);
    if (statusDetail) {
        delete statusDetail;
        statusDetail = nullptr;
    }

    if (status.good() && rspStatus == STATUS_Success) {
        cout << "C-ECHO successful! (msgId: " << msgId << ")" << endl;
        return true;
    }
    cerr << "C-ECHO failed: " << status.text() << " (Status: " << rspStatus << ", msgId: " << msgId << ")" << endl;
    return false;
}

struct QueryResult {
    std::string patientName;
    std::string studyUID;
};

void findCallback(void *callbackData, T_DIMSE_C_FindRQ *request, int responseCount,
                  T_DIMSE_C_FindRSP *response, DcmDataset *rspIds) {
    if (rspIds) {
        auto *foundFiles = static_cast<std::vector<std::string>*>(callbackData);

        OFString studyUID;
        if (rspIds->findAndGetOFString(DCM_StudyInstanceUID, studyUID).good()) {
            foundFiles->push_back(studyUID.c_str());
        }
    }
}



bool DICOMClient::sendCFind(
    const int msgId,
    DcmDataset &query,
    vector<string> &foundFiles,
    int &responseCount) const {

    T_DIMSE_C_FindRQ request{};
    request.MessageID = msgId;
    request.DataSetType = DIMSE_DATASET_PRESENT;
    strcpy(request.AffectedSOPClassUID, UID_FINDPatientRootQueryRetrieveInformationModel);

    T_DIMSE_C_FindRSP response{};

    const T_ASC_PresentationContextID presID = ASC_findAcceptedPresentationContextID(assoc, request.AffectedSOPClassUID);
    if (presID == 0) {
        cerr << "No suitable presentation context for C-FIND!" << endl;
        return false;
    }

    DcmDataset *statusDetail = nullptr;
    responseCount = 0;

    const OFCondition cond = DIMSE_findUser(
        assoc, presID, &request, &query, responseCount,
        findCallback, &foundFiles,
        DIMSE_BLOCKING, 90, &response, &statusDetail);

    if (statusDetail) {
        delete statusDetail;
        statusDetail = nullptr;
    }

    if (cond.good()) {
        cout << "Response #" << responseCount - 1<< " received. "<< endl;
        cout << "Found: " << foundFiles.size() << endl;
        return true;
    }
    cerr << "C-FIND failed: " << cond.text() << endl;
    return false;
}

bool DICOMClient::sendCStore(int msgId, const string &dicomFilePath) const {
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

    T_DIMSE_C_StoreRQ request{};

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


    const T_ASC_PresentationContextID presId = ASC_findAcceptedPresentationContextID(assoc, request.AffectedSOPClassUID);
    if (presId == 0) {
        cerr << "No suitable presentation context for C-STORE!" << endl;
        return false;
    }

    T_DIMSE_C_StoreRSP response{};

    DcmDataset *statusDetail = nullptr;
    status = DIMSE_storeUser(assoc, presId, &request,
                            dicomFilePath.c_str(), dataset,
                            nullptr,nullptr, DIMSE_BLOCKING,
                            0, &response, &statusDetail,
                            nullptr, 0);
    if (statusDetail) {
        delete statusDetail;
        statusDetail = nullptr;
    }
    if (status.good() && response.DimseStatus == STATUS_Success) {
        cout << "C-STORE successful! (msgId: " << msgId << ", File: " << dicomFilePath << ")" << endl;
        return true;
    }
    cerr << "C-STORE failed: " << status.text() << " (Status: " << response.DimseStatus << ", msgId: " << msgId << ")" << endl;
    return false;
}

DcmDataset DICOMClient::createFindQuery(const string &patientName,
                                        const string &patientID,
                                        const string &studyDate,
                                        const string &modality,
                                        const string &accessionNumber) {
    DcmDataset query;
    query.putAndInsertString(DCM_QueryRetrieveLevel, "PATIENT");

    query.putAndInsertString(DCM_PatientName, patientName.empty() ? "*" : patientName.c_str());
    query.putAndInsertString(DCM_PatientID, patientID.empty() ? "*" : patientID.c_str());
    query.putAndInsertString(DCM_StudyDate, studyDate.empty() ? "" : studyDate.c_str());
    query.putAndInsertString(DCM_Modality, modality.empty() ? "*" : modality.c_str());
    query.putAndInsertString(DCM_AccessionNumber, accessionNumber.empty() ? "*" : accessionNumber.c_str());
    query.putAndInsertString(DCM_StudyInstanceUID, "*");  // Thêm StudyInstanceUID
    query.putAndInsertString(DCM_SOPInstanceUID, "*");    // Thêm SOPInstanceUID

    return query;
}
