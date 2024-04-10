#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <cstdint>
#include <cassert>
#include <vector>
#include <iomanip>
#include <stdio.h>

template<typename T>
std::istream& raw_read(std::istream& is, T& val, size_t size = sizeof(T)) {
	return is.read(reinterpret_cast<char*>(&val), size);
}

using namespace std;

struct field {
	uint8_t number_;
	uint8_t size_;
	uint8_t base_;
};

struct definition_msg {
	uint8_t reserved_;
	uint8_t architecture_;
	uint16_t G_ID_; //global id
	uint8_t num_fields_; 
	uint8_t local_ID_;
	std::vector<field> fields_;

	void copy(const definition_msg& d) {
		reserved_ = d.reserved_;
		architecture_ = d.architecture_;
		G_ID_ = d.G_ID_;
		num_fields_ = d.num_fields_;
		local_ID_ = d.local_ID_;
		fields_.resize(d.fields_.size());
		std::copy(d.fields_.begin(), d.fields_.end(), fields_.begin());
	}
	
	uint32_t length() {
		uint32_t size = 0;
		for (auto x : fields_) {
			size += x.size_;
		}
		return size;
	}
};

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

template<typename T>
void get_val(uint16_t& crc, std::istream& is, int n, T& val) {
	uint8_t byte;
	T j = 0;
	val = 0;
	for (size_t r = 0; r < n; ++r) {
		raw_read(is, byte);
		FitCRC_Get16(crc, byte);
		j = byte;
		j = (j << 8 * r);
		val = val | j;
	}
}

bool read_header(std::istream& is, uint32_t& dsize) {
	uint8_t hsize;
	uint8_t protocol_version;
	uint16_t profile_version;
	uint8_t dtype[4]; //".FIT"
	uint16_t hCRC;
	uint16_t crc = 0;

	//header file
	raw_read(is, hsize);
	FitCRC_Get16(crc, hsize);
	raw_read(is, protocol_version);
	FitCRC_Get16(crc, protocol_version);

	get_val(crc, is, 2, profile_version);
	get_val(crc, is, 4, dsize);

	raw_read(is, dtype);
	for (size_t i = 0; i < 4; ++i) {
		FitCRC_Get16(crc, dtype[i]);
	}

	raw_read(is, hCRC);

	if (crc == hCRC) {
		cout << "Header CRC ok\n";
		return true;
	}
	else return false;
}

void read_data(const uint32_t& n, std::istream& is, uint16_t &crc) {
	uint8_t byte;
	for (size_t r = 0; r < n; ++r) {
		raw_read(is, byte);
		FitCRC_Get16(crc, byte);
	}
}

void trova_dmsg(std::vector<definition_msg> dict, uint8_t local_ID, definition_msg& d) {
	for (auto x : dict) {
		if (local_ID == x.local_ID_) {
			d.copy(x);
			return;
		}
	}
}

int fit_dump(std::istream& is) {
	uint32_t dsize; //in bytes
	uint16_t crc = 0;
	uint8_t header;
	uint8_t byte = 0;
	uint8_t type = 0;
	uint8_t local_ID = 0;
	bool first = true;
	size_t dim = 0;
	std::vector<definition_msg> dict;

	//header
	if (!read_header(is, dsize)) return 1;

	crc = 0;
	//data records
	while (dim < dsize) {
		raw_read(is, header);
		FitCRC_Get16(crc, header);
		dim++;
		type = (header >> 4) & 0X0F;
		local_ID = header & 0x0F; 
		if (type == 4) { // definition msg
			definition_msg dmsg;
			dmsg.local_ID_ = local_ID;

			raw_read(is, dmsg.reserved_); //reserved
			FitCRC_Get16(crc, dmsg.reserved_);

			raw_read(is, dmsg.architecture_);//architecture
			FitCRC_Get16(crc, dmsg.architecture_);

			uint16_t G_ID = 0; //G_ID
			get_val(crc, is, 2, G_ID);
			dmsg.G_ID_ = G_ID;

			raw_read(is, dmsg.num_fields_); //num_fields
			FitCRC_Get16(crc, dmsg.num_fields_);

			field f;
			dim = dim + 5 + (dmsg.num_fields_ * 3); //dimensione di tutto il definition msg

			//leggo num_fields campi
			for (size_t i = 0; i < dmsg.num_fields_; ++i) {
				raw_read(is, f.number_);
				FitCRC_Get16(crc, f.number_);

				raw_read(is, f.size_);
				FitCRC_Get16(crc, f.size_);

				raw_read(is, f.base_);
				FitCRC_Get16(crc, f.base_);

				dmsg.fields_.push_back(f);
			}
			dict.push_back(dmsg);
		}
		else if (type == 0) { //datamsg
			//local_ID
			definition_msg dmsg;
			trova_dmsg(dict, local_ID, dmsg);
			uint32_t length = dmsg.length(); //numero byte da leggere
			dim += length;
			if (first) { //primo record
				for (size_t i = 0; i < dmsg.fields_.size(); ++i) {
					if (dmsg.fields_[i].number_ == 4) { // time
						uint32_t time = 0;
						get_val(crc, is, 4, time);
						cout << "time_created = " << time << "\n";
					}
					else read_data(dmsg.fields_[i].size_, is, crc);
				}
				first = false;
			}
			else if (dmsg.G_ID_ == 19) { // global message type 19 
				for (size_t i = 0; i < dmsg.fields_.size(); ++i) {
					if (dmsg.fields_[i].number_ == 13) { // avg_speed
						uint16_t avg_speed = 0;
						get_val(crc, is, 2, avg_speed);
						cout << "avg_speed = " << std::setprecision(4) << avg_speed * 0.0036 << "km/h\n";
					}
					else read_data(dmsg.fields_[i].size_, is, crc);
				}
			}
			else { // dati a caso
				read_data(length, is, crc);
			}
		}
		else return 1;
	}
	//CRC
	uint16_t CRC;
	raw_read(is, CRC);
	if (crc == CRC) {
		cout << "File CRC ok\n";
		return 0;
	}
	else return 1;
}

int main(int argc, char** argv)
{
	if (argc != 2) return 1;
	std::ifstream is(argv[1], std::ios_base::binary);
	if (!is) return 1;

	return fit_dump(is);
}