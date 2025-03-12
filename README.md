# DCMQRSCP
```bash
export DCMDICTPATH="/mnt/c/usr/local/share/dicom.dic"
```

```bash
dcmqrscp -d -c dcmqrscp.cfg  1024
```

```cfg
NetworkTCPPort = 1024
MaxPDUSize = 16384
MaxAssociations = 16

HostTable BEGIN
PACS1 = (PACS_SRC1, localhost, 1024)
HostTable END

AETable BEGIN
PACS_SRC /mnt/c/HoangTu/Programing/DicomGateWay/dcmqrscp/dicom_database RW (100, 1024mb) ANY
AETable END

```
PACS_SRC == aeSCP

# STORESCP
```bash
storescp -d +xa -od /mnt/c/HoangTu/Programing/DicomGateWay/StoreSCPData 1024
```
