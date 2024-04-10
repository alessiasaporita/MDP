#include <fstream>
#include <array>
#include <vector>
#include <string>
#include <cstdint>

template <typename T>
class mat {
	int rows_ = 0, cols_ = 0;
	std::vector<T> data_;

public:
	mat() {	}
	mat(int rows, int cols): rows_(rows), cols_(cols), data_(rows*cols){}

	int rows() const { return rows_; }
	int cols() const { return cols_; }

	void resize(int rows, int cols) {
		rows_ = rows;
		cols_ = cols;
		data_.resize(rows * cols);
	}

	T& operator()(int r, int c) {
		return data_[r * cols_ + c];
	}
	const T& operator()(int r, int c) const {
		return data_[r * cols_ + c];
	}

	char* raw_data() {
		return reinterpret_cast<char*>(data_.data());
	}
	const char* raw_data() const{
		return reinterpret_cast<const char*>(data_.data());
	}
	int raw_size() { return rows_ * cols_ * sizeof(T); }
};

using vec4b = std::array<uint8_t, 4>;


bool read_PAM(std::istream& is, mat<vec4b>& img) {
	std::string magic_number, token, tupletype;
	int h, w, depth = 0, maxval = 0;

	is >> magic_number;
	if (magic_number != "P7") return false;
	if (is.get() != '\n') return false;
	if (is.peek() == '#') {
		while (is.get() != '\n') {}
	}
	while (1) {
		is >> token;
		if (token == "HEIGHT") { is >> h; }
		else if (token == "WIDTH") { is >> w; }
		else if (token == "DEPTH") { is >> depth; }
		else if (token == "MAXVAL") { is >> maxval; }
		else if (token == "TUPLTYPE") { is >> tupletype; }
		else if (token == "ENDHDR") { break; }
		else return false;
		if (is.get() != '\n') return false;
	}
	if (is.get() != '\n') return false;
	if (maxval != 255) return false;
	if (tupletype != "RGB" && tupletype != "RGB_ALPHA") return false;
	if (depth != 4 && depth != 3) return false;
	img.resize(h, w);
	for (int r = 0; r < img.rows(); ++r) {
		for (int c = 0; c < img.cols(); ++c) {
			img(r, c)[0] = is.get();
			img(r, c)[1] = is.get();
			img(r, c)[2] = is.get();
			if (depth == 3) {
				img(r, c)[3] = 255;
			}
			else {
				img(r, c)[3] = is.get();
			}
		}
	}
	return true;
}


bool save_PAM(std::ostream& os, mat<vec4b>& img) {
	std::string magic_number, token, tupletype;
	int h, w, depth = 0, maxval = 0;

	os << "P7\n";
	os << "WIDTH " << img.cols() << "\n";
	os << "HEIGHT " << img.rows() << "\n";
	os << "DEPTH 4\n";
	os << "MAXVAL 255\n";
	os << "TUPLTYPE RGB_ALPHA\n";
	os << "ENDHDR\n";

	for (int r = 0; r < img.rows(); ++r) {
		for (int c = 0; c < img.cols(); ++c) {
			os.put(img(r, c)[0]); 
			os.put(img(r, c)[1]); 
			os.put(img(r, c)[2]); 
			os.put(img(r, c)[3]); 
		}
	}
	return true;
}

void paste(const mat<vec4b>& src, mat<vec4b>& dst, int x, int y) {
	//foregroundA = src, backgroundB = dst
	double a0 = 0, aa = 0, ab = 0;
	double red = 0, blue = 0, green = 0;
	for (int r = 0; r < src.rows(); ++r) {
		for (int c = 0; c < src.cols(); ++c) {
			int X = c + x;
			int Y = r + y;
			aa = src(r, c)[3] / 255.0;
			ab = dst(Y, X)[3] / 255.0;
			a0 = aa + ab * (1.0 - aa);
			
			red = (src(r, c)[0] * aa + dst(Y, X)[0] * ab * (1.0 - aa)) / a0;
			green = (src(r, c)[1] * aa + dst(Y, X)[1] * ab * (1.0 - aa)) / a0;
			blue = (src(r, c)[2] * aa + dst(Y, X)[2] * ab * (1.0 - aa)) / a0;			
			a0 = a0 * 255.0;

			//saturo rgba
			red = (red > 255) ? 255 : red;
			green = (green > 255) ? 255 : green;
			blue = (blue > 255) ? 255 : blue;
			a0 = (a0 > 255) ? 255 : a0;

			dst(Y, X)[0] = (uint8_t)red;
			dst(Y, X)[1] = (uint8_t)green;
			dst(Y, X)[2] = (uint8_t)blue;
			dst(Y, X)[3] = (uint8_t)(a0);
		}
	}
}

struct coord {
	int x = 0;
	int y = 0;
};

int main(int argc, char* argv[]) {
	std::string form = ".pam", name;
	mat<vec4b> output;
	std::vector<mat<vec4b>> images;
	std::vector<coord> coords;
	name = argv[1] + form;
	coord pos;
	int x = 0, y = 0;
	int X = 0, Y = 0; //size output finale

	std::ofstream os(name, std::ios_base::binary);
	if (!os) return EXIT_FAILURE;

	for (int i = 2; i < argc; ++i) {
		//apro img da incollare
		name = argv[i];
		if (name == "-p") {
			++i;
			x = std::stoi(argv[i]);
			++i;
			y = std::stoi(argv[i]);
			++i;
			pos.x = x;
			pos.y = y;
		}
		else {
			pos.x = 0;
			pos.y = 0;
		}

		name = argv[i] + form;
		std::ifstream is(name, std::ios_base::binary);
		if (!is) return EXIT_FAILURE;
		mat<vec4b> img;
		read_PAM(is, img);

		//inserisco nel vettore
		images.push_back(img);
		coords.push_back(pos);

		//resize
		if (x + img.cols() > X) {
			X = x + img.cols();
		}
		if (y + img.rows() > Y) {
			Y = y + img.rows();
		}
	}

	output.resize(Y, X);

	for(size_t i=0; i<images.size(); ++i){
		//incollo
		paste(images[i], output, coords[i].x, coords[i].y);
	}
	save_PAM(os, output);
	return 0;
}
