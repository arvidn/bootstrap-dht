/*
The MIT License (MIT)

Copyright (c) 2019 Arvid Norberg

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
the Software, and to permit persons to whom the Software is furnished to do so,
subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include <iostream>
#include <boost/asio.hpp>
#include <set>

using boost::asio::ip::address_v4;

std::set<address_v4> duplicates;

int main(int argc, char* argv[])
{
	if (argc != 2)
	{
		std::cerr << "Usage: print_ips <filename>\n";
		return 1;
	}

	FILE* f = fopen(argv[1], "rb");
	if (f == nullptr)
	{
		std::cerr << "failed to open file: " << strerror(errno) << '\n';
		return 1;
	}

	address_v4::bytes_type b;
	for (;;)
	{
		auto ret = fread(b.data(), b.size(), 1, f);
		if (ret != 1) break;
		address_v4 ip(b);
		if (duplicates.insert(ip).second == false) continue;

		std::uint16_t time = 0;
		ret = fread(&time, sizeof(time), 1, f);
		if (ret != 1) break;

		std::array<std::uint8_t, 4> client_id;
		ret = fread(client_id.data(), client_id.size(), 1, f);
		if (ret != 1) break;

		std::cout << int(time) << ' ' << ip.to_string() << ' '
			<< client_id[0] << client_id[1] << '-' << int(client_id[2])
			<< '.' << int(client_id[3]) << '\n';
	}

	fclose(f);
}
