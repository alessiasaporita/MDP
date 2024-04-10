#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <cstdint>
#include <cassert>
#include <cstring>
#include <iomanip>

using namespace std;

template <typename T>
istream& raw_read(istream& is, T& val, size_t size = sizeof(T)) {
	return is.read(reinterpret_cast<char*>(&val), size);
}

void FitCRC_Get16(uint16_t& crc, uint8_t byte)
{
	static const uint16_t crc_table[16] =
	{
		0x0000, 0xCC01, 0xD801, 0x1400, 0xF001, 0x3C00, 0x2800, 0xE401,
		0xA001, 0x6C00, 0x7800, 0xB401, 0x5000, 0x9C01, 0x8801, 0x4400
	};
	uint16_t tmp;
	// compute checksum of lower four bits of byte
	tmp = crc_table[crc & 0xF];
	crc = (crc >> 4) & 0x0FFF;
	crc = crc ^ tmp ^ crc_table[byte & 0xF];
	// now compute checksum of upper four bits of byte
	tmp = crc_table[crc & 0xF];
	crc = (crc >> 4) & 0x0FFF;
	crc = crc ^ tmp ^ crc_table[(byte >> 4) & 0xF];
}
struct header {
	uint8_t fsize;
	uint8_t version;
	uint16_t profile;
	uint32_t dsize;
	char type[4];
	uint16_t hcrc;
};

bool read_header(istream& is, header& h) {
	h.fsize = is.get();
	h.version = is.get();
	raw_read(is, h.profile);
	raw_read(is, h.dsize);
	raw_read(is, h.type);
	raw_read(is, h.hcrc);
	if (strncmp(h.type, ".FIT", 4) != 0) return false;
	return true;
}

struct field {
	uint8_t number;
	uint8_t size;
	uint8_t type;
};

struct definition_msg {
	uint8_t reserved;
	uint8_t architecture;
	uint16_t global_msg_number;
	uint8_t numfields;
	std::vector<field> fields;
};

int fitdump(istream& is) {
	header h;
	if (!read_header(is, h)) return 1;
	uint16_t hcrc = 0;
	uint8_t byte;
	FitCRC_Get16(hcrc, h.fsize);
	FitCRC_Get16(hcrc, h.version);

	FitCRC_Get16(hcrc, (uint8_t)h.profile);
	byte = (h.profile >> 8);
	FitCRC_Get16(hcrc, byte);

	FitCRC_Get16(hcrc, h.dsize);
	byte = (h.dsize >> 8);
	FitCRC_Get16(hcrc, byte);
	byte = (h.dsize >> 16);
	FitCRC_Get16(hcrc, byte);
	byte = (h.dsize >> 24);
	FitCRC_Get16(hcrc, byte);

	for (int i = 0; i < 4; ++i) {
		FitCRC_Get16(hcrc, h.type[i]);
	}

	if (hcrc == h.hcrc) {
		cout << "Header CRC ok\n";
	}
	else {
		return 1;
	}

	map<uint8_t, definition_msg> dict;
	hcrc = 0;
	bool first = true;
	uint32_t count = 0;
	while (count<h.dsize) {
		byte = is.get();
		FitCRC_Get16(hcrc, byte);
		count++;
		if (((byte >> 4) & 0b00001111) == 4) { //definition msg
			definition_msg dmsg;
			field f;
			raw_read(is, dmsg.reserved);
			raw_read(is, dmsg.architecture);
			raw_read(is, dmsg.global_msg_number);
			raw_read(is, dmsg.numfields);
			count += 5;
			//aggiorno crc
			FitCRC_Get16(hcrc, dmsg.reserved);
			FitCRC_Get16(hcrc, dmsg.architecture);
			FitCRC_Get16(hcrc, dmsg.global_msg_number);
			FitCRC_Get16(hcrc, (dmsg.global_msg_number >> 8));
			FitCRC_Get16(hcrc, dmsg.numfields);

			for (size_t i = 0; i < dmsg.numfields; ++i) {
				raw_read(is, f.number);
				raw_read(is, f.size);
				raw_read(is, f.type);
				count += 3;
				//aggiorno crc
				FitCRC_Get16(hcrc, f.number);
				FitCRC_Get16(hcrc, f.size);
				FitCRC_Get16(hcrc, f.type);

				dmsg.fields.push_back(f);
			}
			uint8_t local_ID = byte & 0b00001111;
			dict[local_ID] = dmsg;
		}
		else if (((byte >> 4) & 0b00001111) == 0) { //data msg
			uint8_t local_ID = byte & 0b00001111;
			auto dfield = dict.find(local_ID);
			if (dfield == dict.end()) return 1; //field non esiste
			definition_msg def_field = dfield->second;
			uint8_t size;
			if (first) {
				for (int i = 0; i < def_field.numfields; ++i) {
					if (def_field.fields[i].number == 4) {
						cout << "time_created = ";
						uint32_t time;
						raw_read(is, time);
						count += 4;
						cout << time << "\n";
						//aggiorno crc
						FitCRC_Get16(hcrc, time);
						FitCRC_Get16(hcrc, (time >> 8));
						FitCRC_Get16(hcrc, (time >> 16));
						FitCRC_Get16(hcrc, (time >> 24));
					}
					else {
						size = def_field.fields[i].size;
						for (int j = 0; j < size; ++j) {
							//aggiorno crc
							FitCRC_Get16(hcrc, is.get());
							count ++;
						}
					}
				}
				first = false;
			}
			else if (def_field.global_msg_number == 19) {
				for (int i = 0; i < def_field.numfields; ++i) {
					if (def_field.fields[i].number == 13) {
						cout << "avg_speed = ";
						uint16_t speed;
						raw_read(is, speed);
						double conversion = 0.0036 * speed;
						cout << std::setprecision(4) << conversion << " km/h\n";
						//aggiorno crc
						FitCRC_Get16(hcrc, speed);
						FitCRC_Get16(hcrc, (speed >> 8));
						count+=2;
					}
					else {
						size = def_field.fields[i].size;
						for (int j = 0; j < size; ++j) {
							//aggiorno crc
							FitCRC_Get16(hcrc, is.get());
							count++;
						}
					}
				}
			}
			else {
				//leggo i campi
				for (int i = 0; i < def_field.numfields; ++i) {
					size = def_field.fields[i].size;
					for (int j = 0; j < size; ++j) {
						FitCRC_Get16(hcrc, is.get());
						count++;
					}
				}
			}
		}
		else {
			break;
		}
	}
	uint16_t crc;
	raw_read(is, crc);
	if (hcrc == crc) {
		cout << "File CRC ok\n";
		return 0;
	}
	else return 1;
}


int main(int argc, char **argv)
{
	ifstream is(argv[1], ios_base::binary);
	if (!is) return 1;
	return fitdump(is);
}