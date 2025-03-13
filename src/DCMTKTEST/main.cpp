#include <dcmtk/dcmdata/dctk.h>
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
  string path = "/mnt/c/HoangTu/Programing/DicomGateWay/DICOMGATEWAY/dcmqrscp/scp01/database/CT_67d24d60621992de.dcm";
  printDICOMInfo(path);
  return 0;
}
