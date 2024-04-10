#include <fstream>
#include <vector>
#include <string>
#include <array>
#include <cstdint>
#include <cmath>

template <typename T>
class mat
{
	int row = 0, col = 0;
	std::vector<T> data;

public:
	mat() {}
	mat(int r, int c) : row(r), col(c), data(r * c) {}

	void resize(int r, int c)
	{
		row = r;
		col = c;
		data.resize(r * c);
	}

	int rows() { return row; }
	int cols() { return col; }

	T &operator()(int r, int c) { return data[r * col + c]; }
	const T &operator()(int r, int c) const { return data[r * col + c]; }

	int rawsize() const { return row * col * sizeof(T); }
	char *raw_data() { return reinterpret_cast<char *>(&data[0]); }
	const char *raw_data() const { return reinterpret_cast<char *>(&data[0]); }
};

//using vec3b = std::array<uint8_t, 3>;
using vec4b = std::array<uint8_t, 4>;

bool load_PAM(std::istream &is, mat<vec4b> &m)
{
	std::string magic_number, s, tupltype;
	int H, W, D, maxval;

	is >> magic_number;
	if (magic_number != "P7")
		return false;
	if (is.get() != '\n') return false;

	is >> s;
	if (s != "WIDTH")
		return false;
	is >> W;
	if (is.get() != '\n') return false;

	is >> s;
	if (s != "HEIGHT")
		return false;
	is >> H;
	if (is.get() != '\n') return false;

	is >> s;
	if (s != "DEPTH")
		return false;
	is >> D;
	if (is.get() != '\n') return false;

	is >> s;
	if (s != "MAXVAL")
		return false;
	is >> maxval;
	if (maxval != 255)
		return false;
	if (is.get() != '\n') return false;

	is >> s;
	if (s != "TUPLTYPE")
		return false;
	is >> tupltype;
	if (is.get() != '\n') return false;

	is >> s;
	if (s != "ENDHDR")
		return false;
	if (is.get() != '\n') return false;

	m.resize(H, W);
	for (int r = 0; r < m.rows(); ++r)
	{
		for (int c = 0; c < m.cols(); ++c)
		{
			m(r, c)[0] = is.get();
			m(r, c)[1] = is.get();
			m(r, c)[2] = is.get();
			if (D == 4) {
				m(r, c)[3] = is.get();
			}
			else {
				m(r, c)[3] = 255;
			}
		}
	}

	//is.read(m.raw_data(), m.rawsize());

	return true;
}

bool save_PAM(std::ostream &os, mat<vec4b> &m)
{
	os << "P7\n";
	os << "WIDTH " << m.cols() << "\n";
	os << "HEIGHT " << m.rows() << "\n";
	os << "DEPTH 4\n";
	os << "MAXVAL 255\n";
	os << "TUPLTYPE RGB_ALPHA\n";
	os << "ENDHDR\n";
	for (int r = 0; r < m.rows(); ++r)
	{
		for (int c = 0; c < m.cols(); ++c)
		{
			vec4b pixel = m(r, c);
			os.write(reinterpret_cast<const char *>(&pixel[0]), 4);
		}
	}
	return true;
}

int main(int argc, char *argv[])
{
	mat<vec4b> mat_output;
	mat<vec4b> m;
	std::string est = ".pam";

	for (size_t i = 2; i < argc; ++i)
	{
		std::string s = argv[i];
		int x, y;
		if (s == "-p")
		{
			x = std::stoi(argv[i + 1]);
			y = std::stoi(argv[i + 2]);
			i += 3;
		}
		else {
			x = 0;
			y = 0;
		}
		std::string name = argv[i] + est;
		std::ifstream is(name, std::ios_base::binary);
		if (!is)
			return 1;

		load_PAM(is, m);
		if (m.rows() + y > mat_output.rows())
		{
			mat_output.resize(m.rows() + y, mat_output.cols());
		}
		if (m.cols() + x > mat_output.cols())
		{
			mat_output.resize(mat_output.rows(), m.cols() + x);
		}
		for (int r = 0; r < m.rows(); ++r)
		{
			for (int c = 0; c < m.cols(); ++c)
			{
				// mat_output(r + y, c + x)[3] == background
				// m(r, c)[3] == foreground
				uint16_t a0 = m(r, c)[3] + mat_output(r + y, c + x)[3] * (1 - m(r, c)[3]);
				if (a0 != 0) {
					mat_output(r + y, c + x)[0] = (m(r, c)[0] * m(r, c)[3] + mat_output(r + y, c + x)[0] * mat_output(r + y, c + x)[3] * (1 - m(r, c)[3])) / a0;
					mat_output(r + y, c + x)[1] = (m(r, c)[1] * m(r, c)[3] + mat_output(r + y, c + x)[1] * mat_output(r + y, c + x)[3] * (1 - m(r, c)[3])) / a0;
					mat_output(r + y, c + x)[2] = (m(r, c)[2] * m(r, c)[3] + mat_output(r + y, c + x)[2] * mat_output(r + y, c + x)[3] * (1 - m(r, c)[3])) / a0;
				}
				else {
					mat_output(r + y, c + x)[0] = m(r, c)[0];
					mat_output(r + y, c + x)[1] = m(r, c)[1];
					mat_output(r + y, c + x)[2] = m(r, c)[2];
				}
				mat_output(r + y, c + x)[3] = a0;
			}
		}
	}
	std::string name_o = argv[1] + est;
	std::ofstream os(name_o, std::ios_base::binary);
	if (!os)
		return 1;
	save_PAM(os, mat_output);
	return 0;
}