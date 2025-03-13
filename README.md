# DCMQRSCP
```bash
export DCMDICTPATH="/mnt/c/usr/local/share/dicom.dic"
```

```bash
dcmqrscp -d -c dcmqrscp.cfg  1024
```

```bash
dcmqrscp -<option> -c <path to config file>  <tcp/icp port>
```

PACS_SRC == aeSCP

# STORESCP
```bash
storescp -d +xa -od /mnt/c/HoangTu/Programing/DicomGateWay/StoreSCPData 1024
```
