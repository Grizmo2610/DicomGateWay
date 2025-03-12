#include "DICOMClient.h"
#include <filesystem>
#include <vector>
#include <random>

namespace fs = std::filesystem;
using namespace std;
using namespace fs;

void testCECHOSCU(DICOMClient client) {
    if (client.connect(UID_VerificationSOPClass, UID_LittleEndianExplicitTransferSyntax, 1)) {
        if (client.sendMessage(1, "")) {
            cout << ">>>C-ECHO OK" << endl;
        } else {
            cout << ">>>C-ECHO FAIL" << endl;
        }
    }else {
        cout << ">>>CONNECTION FAIL" << endl;
    }
}

string random_path(const string& folderPath = "/mnt/c/HoangTu/Programing/DicomGateWay/DICOMGATEWAY/DCMGATE/data/CT3/1.2.840.113704.9.1000.16.1.2024100908464206400020001") {
    vector<path> files;

    if (!exists(folderPath) || !is_directory(folderPath)) {
        cerr << "Error: Folder does not exist or is not a directory!" << endl;
        return "";
    }

    for (const auto &entry : directory_iterator(folderPath)) {
        if (entry.is_regular_file()) {
            files.push_back(entry.path());
        }
    }

    if (files.empty()) {
        cerr << "Error: Folder is empty!" << endl;
        return "";
    }

    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<size_t> dist(0, files.size() - 1);

    return files[dist(gen)].string();
}

void testCSTORESCU(DICOMClient client) {
    const char *abstractSyntax = UID_CTImageStorage;
    const char *transferSyntax = UID_LittleEndianExplicitTransferSyntax;

    if (client.connect(abstractSyntax, transferSyntax)) {
        if (const auto path = random_path(); client.sendMessage(3, path)) {
            cout << ">>>C-STORE OK" << endl;
        } else {
            cout << ">>>C-STORE FAIL" << endl;
        }
    } else {
        cout << ">>>CONNECTION FAIL" << endl;
    }
}

void testCFINDSCU(DICOMClient client) {
    client.disconnect();
    const char *abstractSyntax = UID_FINDPatientRootQueryRetrieveInformationModel;
    const char *transferSyntax = UID_LittleEndianImplicitTransferSyntax;

    if (client.connect(abstractSyntax, transferSyntax)) {
        if (client.sendMessage(2, "")) {
            cout << ">>>C-FIND OK" << endl;
        } else {
            cout << ">>>C-FIND FAIL" << endl;
        }
    } else {
        cout << ">>>CONNECTION FAIL" << endl;
    }
}


int main() {
    DICOMClient client;
    cout << "===TEST C-ECHO SCU===" << endl;
    testCECHOSCU(client);
    cout << "======================" << endl;

    cout << "===TEST C-STORE SCU===" << endl;
    testCSTORESCU(client);
    cout << "======================" << endl;

    cout << "===TEST C-FIND SCU===" << endl;
    testCFINDSCU(client);
    cout << "======================" << endl;

    client.disconnect();
    return 0;
}
