#include <iostream>
#include <random>
#include <algorithm>
#include <fstream>
#include <iterator>


std::ifstream open(const char* path, std::ios_base::openmode mode = std::ios_base::in) {
  std::ifstream file{ path, mode };
  if(!file.is_open()) {
    std::string err{ "Unable to open file " };
    err.append(path);
    throw std::runtime_error{ err };
  }
  file.exceptions(std::ifstream::badbit);
  return file;
} // Chapter 16!


template <typename AudienceType> // Chapter 6!
std::vector<AudienceType> make_audience(const char* path="audience.txt") {
  auto audience_file = open(path); // Chapter 13!
  std::vector<AudienceType> audience;
  std::copy(std::istream_iterator<AudienceType>{ audience_file },
       std::istream_iterator<AudienceType>{},
            std::back_inserter(audience)); // Chapters 14, 16, and 18!
  return audience;
}

int main() {
  const size_t number_of_winners{ 3 };
  try { // Chapter 4!
    auto audience = make_audience<unsigned int>();
    std::shuffle(std::begin(audience), std::end(audience), std::random_device{}); // Chapter 18!
    for(size_t index{}; index < std::min(std::size(audience), number_of_winners); index++)
      std::cout << "Winner " << index+1 << ": " << audience[index] << "\n";
  } catch(std::exception& e) {
    std::cerr << "[-] Error: " << e.what() << "\n";
  }
}
