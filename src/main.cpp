#include "DICOMClient.h"

void testCECHOSCU(DICOMClient client) {
    const char *abstractSyntax = UID_VerificationSOPClass;
    const char *transferSyntax = UID_LittleEndianImplicitTransferSyntax;

    if (client.connect(abstractSyntax, transferSyntax)) {
        if (client.sendMessage(1, "")) {
            cout << "Echo OK" << endl;
        }else {
            cout << "Echo FAIL" << endl;
        }
    }
}

void testCSTORESCU(DICOMClient client) {
    const char *abstractSyntax = UID_CTImageStorage;
    const char *transferSyntax = UID_LittleEndianExplicitTransferSyntax;

    if (client.connect(abstractSyntax, transferSyntax)) {
        const auto path = "/mnt/c/HoangTu/Programing/DicomGateWay/DICOMGATEWAY/DCMGATE/data/first.dcm";
        if (client.sendMessage(3, path)) {
            cout << "STORE OK" << endl;
        }else {
            cout << "STORE FAIL" << endl;
        }
    }else {
        cout << "CONNECTION FAIL" << endl;
    }
}

int main() {
    DICOMClient client;
    cout << "===TEST C-ECHO SCU===" << endl;
    testCECHOSCU(client);
    cout << "=== === === === === ===" << endl;

    cout << "===TEST C-STORE SCU===" << endl;
    testCSTORESCU(client);
    cout << "=== === === === === ===" << endl;

    client.disconnect();
    return 0;
}
