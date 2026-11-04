#ifndef HWID_PC_H
#define HWID_PC_H

#include <winsock2.h>
#include <iphlpapi.h>
#include <Windows.h>
#include <QString>

namespace HWID {
    class hwid_client {
        private:
            #pragma pack(1)
            // заголовок SMBIOS
            typedef struct RawSMBIOSData {
                BYTE  Used20CallingMethod;
                BYTE  SMBIOSMajorVersion;
                BYTE  SMBIOSMinorVersion;
                BYTE  DmiRevision;
                DWORD Length;
                BYTE  SMBIOSTableData[];
            } RAW_SMBIOS_DATA, *PRAW_SMBIOS_DATA;


            typedef struct SMBIOSHeader {
                BYTE Type;
                BYTE Length;
                WORD Handle;
            } SMBIOS_HEADER, *PSMBIOS_HEADER;


            typedef struct Type1SystemInformation {
                SMBIOS_HEADER header;
                BYTE manufacturer;
                BYTE product_name;
                BYTE version;
                BYTE serial_number; // Смещение 7 байт от начала структуры

            } TYPE_1_SYSTEM_INFORMATION, *PTYPE_1_SYSTEM_INFORMATION;
            #pragma pack()

            std::string GetDmiString(const SMBIOSHeader* pHeader, BYTE stringIndex) {
                if (stringIndex == 0) return "Not Specified";
                const char* pString = reinterpret_cast<const char*>(pHeader) + pHeader->Length;
                while (--stringIndex > 0 && *pString) {
                    pString += strlen(pString) + 1;
                }
                return (*pString) ? std::string(pString) : "BAD_INDEX";
            }

            QString GetMotherboardSerialNumber() {
                DWORD bufferSize = GetSystemFirmwareTable(0x42534D52, 0, nullptr, 0);
                if (bufferSize == 0) {
                    return "";
                }

                std::vector<BYTE> buffer(bufferSize);
                DWORD result = GetSystemFirmwareTable(0x42534D52, 0, buffer.data(), bufferSize);
                if (result != bufferSize) {
                    return "";
                }

                RawSMBIOSData* pRawData = reinterpret_cast<RawSMBIOSData*>(buffer.data());
                BYTE* pData = pRawData->SMBIOSTableData;
                const BYTE* pEnd = buffer.data() + bufferSize;

                while (pData < pEnd) {
                    SMBIOSHeader* pHeader = reinterpret_cast<SMBIOSHeader*>(pData);
                    if (pHeader->Length < 4) break;

                    if (pHeader->Type == 1 && pHeader->Length >= 0x19) {
                        Type1SystemInformation* pType1 = reinterpret_cast<Type1SystemInformation*>(pData);
                        std::string serial = GetDmiString(pHeader, pType1->serial_number);
                        if (!serial.empty() && serial != "Not Specified" && serial != "BAD_INDEX") {
                            QString serial_data = serial.c_str();
                            return serial_data;
                        }
                        break;
                    }

                    pData += pHeader->Length;
                    while (pData < pEnd && (pData[0] != 0 || pData[1] != 0)) {
                        pData++;
                    }
                    pData += 2;
                }

                return "";
            }



            std::unique_ptr<BYTE[]> adapter_raw_data;
            PIP_ADAPTER_INFO adapter_info;

            QString mac_address;
            QString router_description;
            QString uuid_motherboard;

        public:
            hwid_client(): adapter_info(0) {
                long unsigned int size_header = sizeof(IP_ADAPTER_INFO);
                adapter_raw_data.reset(new BYTE[size_header]);
                adapter_info = reinterpret_cast<PIP_ADAPTER_INFO>(adapter_raw_data.get());

                DWORD code_success = GetAdaptersInfo(adapter_info, &size_header);

                if (code_success == NO_ERROR) {
                    mac_address = reinterpret_cast<const char *>(adapter_info->Address);
                    router_description = adapter_info->Description;
                }

                else if (code_success == ERROR_BUFFER_OVERFLOW) {
                    adapter_raw_data.reset(new BYTE[size_header]);
                    adapter_info = reinterpret_cast<PIP_ADAPTER_INFO>(adapter_raw_data.get());

                    GetAdaptersInfo(adapter_info, &size_header);
                    mac_address = reinterpret_cast<const char *>(adapter_info->Address);
                    router_description = adapter_info->Description;
                }

                uuid_motherboard = GetMotherboardSerialNumber();


            }

            QByteArray get_mac_addr() {
                return mac_address.toUtf8();
            }

            QByteArray get_motherboard_number() {
                return uuid_motherboard.toUtf8();
            }

            QByteArray get_description_adapter() {
                return router_description.toUtf8();
            }
    };
}
#endif // HWID_PC_H
