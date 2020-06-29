/*
The MIT License (MIT)

Copyright (c) 2020 Arvid Norberg

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
#include <iomanip>
#include <set>
#include <map>
#include <vector>
#include <chrono>
#include <array>
#include <sstream>
#include <algorithm>
#include <string_view>
#include <string>

#include <cstdlib>
#include <cstring>

using namespace std::literals::string_view_literals;

using client_id_t = std::array<std::uint8_t, 4>;

std::string print_client(client_id_t const& c)
{
	if (c == client_id_t{{0,0,0,0}}) return "unknown";

	std::string_view const name(reinterpret_cast<char const*>(c.data()), 2);

	// based on http://www.simonwaite.com/projects/bittorrentdht
	// https://github.com/the8472/mldht/blob/master/src/lbms/plugins/mldht/kad/DHTConstants.java
	static std::map<std::string_view, std::string_view> const known_clients = {
		{"LT"sv, "libtorrent"sv},
		{"ml"sv, "mldht"sv},
		{"Az"sv, "Azureus"sv},
		{"AZ"sv, "Azureus"sv},
		{"A2"sv, "Alpha2"sv},
		{"UT"sv, "uTorrent"sv},
		{"MP"sv, "MooPolice"sv},
		{"GR"sv, "GetRight"sv},
		{"MO"sv, "MonoTorrent"sv},
		{"TR"sv, "Transmission"sv},
		{"BF"sv, "BravoFoxtrot"sv},
		{"BT"sv, "BravoTango"sv},
		{"DP"sv, "DhtPlay"sv},
		{"lt"sv, "libtorrent (rakshasa)"sv},
	};

	std::stringstream ret;
	auto const it = known_clients.find(name);
	if (it != known_clients.end())
		ret << it->second;
	else
		ret << char(c[0]) << char(c[1]);

	ret << '-';

	if (name == "LT"sv)
	{
		if (c[2] == 1 && c[3] <= 16)
		{
			// special case for old version of libtorrent
			std::uint32_t const version = c[2];
			std::uint32_t const minor = c[3];
			ret << version << '.' << minor;
		}
		else
		{
			std::uint32_t const version = c[2];
			std::uint32_t const minor = c[3] >> 4;
			std::uint32_t const tag = c[3] & 0xf;
			ret << version << '.' << minor << '.' << tag;
		}
	}
	else
	{
		std::uint32_t const version = (std::uint32_t(c[2]) << 8) | std::uint32_t(c[3]);
		ret << version;
	}

	return ret.str();
}

int main(int argc, char* argv[])
{
	if (argc != 2)
	{
		std::cerr << "Usage: print_stats <filename>\n";
		return 1;
	}

	FILE* f = fopen(argv[1], "rb");
	if (f == nullptr)
	{
		std::cerr << "failed to open file: " << std::strerror(errno) << '\n';
		return 1;
	}

	for (;;)
	{
		std::uint32_t timestamp;
		auto ret = fread(&timestamp, sizeof(timestamp), 1, f);
		if (ret != 1) break;

		std::cout << (timestamp * 3600) << '\n';

		std::uint32_t count;
		ret = fread(&count, sizeof(count), 1, f);
		if (ret != 1) break;

		std::vector<std::pair<int, client_id_t>> sorted;
		for (std::int64_t i = 0; i < count; ++i)
		{
			client_id_t client_id;
			ret = fread(client_id.data(), client_id.size(), 1, f);
			if (ret != 1) break;

			int freq;
			ret = fread(&freq, sizeof(freq), 1, f);
			if (ret != 1) break;

			sorted.emplace_back(freq, client_id);
		}

		sort(sorted.begin(), sorted.end(), std::greater<>());

		for (auto c : sorted)
			std::cout << "  " << std::setw(8) << c.first << ": " << print_client(c.second) << '\n';
	}

	fclose(f);
}
