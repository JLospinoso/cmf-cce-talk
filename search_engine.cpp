#include <iostream>
#include <algorithm>
#include <fstream>
#include <iterator>
#include <string_view>
#include <array>
#include <sstream>
#include <tuple>
#include <unordered_map>
#include <regex>
#include <cstring>
#include <chrono>
#include <atomic>
#include <csignal>

std::atomic_bool interrupted{};
extern "C" void handler(int signal) {
  std::cout << "Handler invoked with signal " << signal << ".\n";
  interrupted = true;
}

void to_lower(std::string& in) {
  for(char& letter : in)
    letter = static_cast<char>(tolower(letter));
}

std::ifstream open(std::string_view path, std::ios_base::openmode mode = std::ios_base::in) {
  std::ifstream file{ path.data(), mode };
  if(!file.is_open()) {
    std::string err{ "Unable to open file " };
    err.append(path);
    throw std::runtime_error{ err };
  }
  file.exceptions(std::ifstream::badbit);
  return file;
}

template <size_t MaxTokens>
struct SearchEngine {
  explicit SearchEngine() : non_word{ R"([^\w\s\d])" } {}

  size_t ingest(std::string_view path) {
    size_t n_tokens_processed{};
    auto document = open(path);
    std::array<std::string, MaxTokens> tokens;
    while(document >> tokens[0]){
      for (size_t token_size{ 1 }; token_size < MaxTokens; token_size++) {
        process_token(tokens, token_size, path);
        ++n_tokens_processed;
      }
      std::move_backward(tokens.begin(), tokens.end()-1, tokens.end());
    }
    return n_tokens_processed;
  }

  [[nodiscard]] std::vector<std::string> search_for(std::string term) const noexcept {
    to_lower(term);
    auto [first_document, last_document] = term_documents_lookup.equal_range(term);
    if (first_document == term_documents_lookup.end())
      return {};
    std::vector<std::string> result;
    std::transform(first_document, last_document,
        std::back_inserter(result),
        [](const auto document){
      auto result = std::get<0>(document.second);
      result.append(" (x");
      result.append(std::to_string(std::get<1>(document.second)));
      result.append(")");
      return result;
    });
    return result;
  }

private:
  void process_token(const std::array<std::string, MaxTokens>& tokens, size_t token_size, std::string_view path) {
    std::stringstream ss;
    std::for_each_n(tokens.rbegin(), token_size, [&ss](const auto& token){
      ss << token << " ";
    });
    auto element = ss.str();
    element.pop_back();
    std::string stripped;
    std::regex_replace(std::back_inserter(stripped), element.begin(), element.end(), non_word, "");
    to_lower(stripped);

    auto [first_document, last_document] = term_documents_lookup.equal_range(stripped);
    if (first_document == term_documents_lookup.end()) {
      term_documents_lookup.insert(
          {stripped, std::tuple<std::string, size_t>{path, 1}});
      return;
    }
    while(first_document != last_document){
      if(std::get<0>(first_document->second) == path){
        ++std::get<1>(first_document->second);
        return;
      }
      ++first_document;
    }
    term_documents_lookup.insert({ stripped, std::tuple<std::string, size_t>{ path, 1 } });
  }
  const std::regex non_word;
  std::unordered_multimap<std::string, std::tuple<std::string, size_t>> term_documents_lookup;
};

template <typename Units>
struct Timer {
  explicit Timer(std::string_view message, std::string_view units)
    : message{ message }, units{ units },
      start{ std::chrono::high_resolution_clock::now() } {}
  ~Timer(){
    const auto as_micros = std::chrono::duration_cast<Units>(std::chrono::high_resolution_clock::now() - start);
    std::cout << message << " took " << as_micros.count() << units << "\n";
  }
private:
  const std::string_view message, units;
  const std::chrono::time_point<std::chrono::high_resolution_clock> start;
};

int main() {
  const static size_t MaxTokenSize { 8 };
  std::vector<std::string> files {
      "pubs/ar350-1.txt",
      "pubs/fm7-8.txt",
      "pubs/jp3-12.txt"
  };
  try {
    std::signal(SIGINT, handler);
    SearchEngine<MaxTokenSize> engine;

    {
      const Timer<std::chrono::milliseconds> processing_timer{ "Processing files", "ms" };
      std::cout << "Processing " << files.size() << " files for search.\n";
      for(const auto& file : files)
        std::cout << "Processed " << engine.ingest(file) << " terms in " << file <<  " \n";
    }

    while(!interrupted) {
      std::cout << "Search term: ";
      std::string term;
      std::getline(std::cin, term);
      std::vector<std::string> results;
      {
        const Timer<std::chrono::microseconds> search_timer{ "Searching", "us" };
        results = engine.search_for(term);
      }
      for(const auto& result : results) {
        std::cout << "\t" << result << "\n";
      }
    };
  } catch(std::exception& e) {
    std::cerr << "Error: " << e.what() << "\n";
  }
}
