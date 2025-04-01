#include <dcmtk/dcmdata/dctk.h>
#include "DICOMClient.h"
#include <iostream>

using namespace std;

void printDICOMInfo(const string &filename) {
  DcmFileFormat fileformat;
  if (fileformat.loadFile(filename.c_str()).good()) {
    DcmDataset *dataset = fileformat.getDataset();
    if (dataset) {
      OFString patientName, patientID, studyDate, modality;

      dataset->findAndGetOFString(DCM_PatientName, patientName);
      dataset->findAndGetOFString(DCM_PatientID, patientID);
      dataset->findAndGetOFString(DCM_StudyDate, studyDate);
      dataset->findAndGetOFString(DCM_Modality, modality);

      cout << "Patient Name: " << patientName << endl;
      cout << "Patient ID: " << patientID << endl;
      cout << "Study Date: " << studyDate << endl;
      cout << "Modality: " << modality << endl;
    }
  } else {
    cerr << "Failed to load DICOM file: " << filename << endl;
  }
}

int main() {
  DcmFileFormat fileFormat;
  DcmDataset *dataset = fileFormat.getDataset();

  setenv("DCMDICTPATH", "/mnt/c/usr/local/share/dicom.dic", 1);

  auto abstractSyntax = UID_FINDPatientRootQueryRetrieveInformationModel;
  auto transferSyntax = UID_LittleEndianImplicitTransferSyntax;

  // string patientName = "CT so nao 16 day [khong tiem]";
  // string patientID = "1909051302";
  string studyDate = "20241009";
  string patientName = "";
  string patientID = "";
  // string studyDate = "";
  string modality = "CT";

  DcmDataset query = DICOMClient::createFindQuery(patientName, patientID, studyDate, modality);
  OFCondition status = fileFormat.saveFile("query.dcm", EXS_LittleEndianExplicit);
  if (status.good()) {
    std::cout << "Đã lưu query.dcm thành công." << std::endl;
  } else {
    std::cerr << "Lỗi khi lưu query.dcm: " << status.text() << std::endl;
  }
  return 0;
}
