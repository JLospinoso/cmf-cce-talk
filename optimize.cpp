#include <iostream>
#include <cmath>
#include <tuple>
#include <string>
#include <string_view>

double func(double arg) {
  return pow(arg, 3.0) - 14.0 * arg - 15.9;
}

double derivative(double x) {
  return 3 * pow(x, 2.0) - 14.0;
}

double ask_for(std::string_view prompt) {
  double result;
  std::cout << prompt << ":\t";
  std::cin >> result;
  std::cout << "\n";
  return result;
}

std::tuple<double, double> calculate_at(double point) {
  return { func(point), derivative(point) };
}

void print(double root, double value, double deriv) {
  printf("x: %10.5g\tfn(x): %10.5g\tfn'(x): %10.5g\n", root, value, deriv);
}

int main() {
  auto epsilon = ask_for("Epsilon");
  if (epsilon < 0) {
    std::cerr << "Epsilon cannot be negative.\n";
    return -1;
  }
  auto root = ask_for("Guess");
  double old_root{};
  do {
    auto [x_val, x_deriv] = calculate_at(root);
    old_root = root;
    root -= x_val / x_deriv;
    print(root, x_val, x_deriv);
  } while (abs(old_root - root) > epsilon);
  std::cout << "Root: " << root << "\n";
  return 0;
}
