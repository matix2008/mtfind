// mtfind.cpp : Этот файл содержит функцию "main". Здесь начинается и заканчивается выполнение программы.
//


//Написать программу mtfind, производящую поиск подстроки в текстовом файле
//по маске с использованием многопоточности.

//Маска - это строка, где "?" обозначает любой символ.
//Программа принимает в качестве параметров командной строки :

//1) Имя текстового файла, в котором должен идти поиск(размер файла - до 1Гб).
//2) Маску для поиска, в кавычках.Максимальная длина маски 100 символов.

//Вывод программы должен быть в следующем формате :
//-На первой строке - количество найденных вхождений.
//- Далее информация о каждом вхождении, каждое на отдельной строке, через пробел :
//номер строки, позиция в строке, само найденное вхождение.
//
//Дополнения :
//	-В текстовом файле кодировка только 7 - bit ASCII
//	- Поиск с учетом регистра
//	- Каждое вхождение может быть только на одной строке.Маска не может содержать символа перевода строки
//	- Найденные вхождения не должны пересекаться.
//	- Пробелы и разделители участвуют в поиске наравне с другими символами.
//	- Можно использовать STL, Boost, возможности С++1x.
//	- Многопоточность нужно использовать обязательно.Однопоточные решения засчитываться не будут.
//	- Серьезным плюсом будет разделение работы между потоками равномерно вне зависимости от количества строк во входном файле.

#include <iostream>
#include <string>
#include <algorithm>

#include <boost/program_options.hpp>
#include <boost/regex.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>

#include "SearchCore.h"

namespace po = boost::program_options;

int main(int argc, char *argv[])
{
	try
	{
		std::string file_;
		std::string mask_;

		po::options_description description("mtfind Usage");

		description.add_options()
			("help,h", "Display this help message")
			("file,f", po::value<std::string>(), "Text file relative path to search substrings.")
			("mask,m", po::value<std::string>(), "Mask to search. Example: \"?ad\"")
			("version,v", "Display the version number");

		po::positional_options_description p;
		p.add("file", -1);

		po::variables_map vm;
		po::store(po::command_line_parser(argc, argv).options(description).positional(p).run(), vm);
		po::notify(vm);

		if (vm.count("help")) {
			std::cout << description;

			return 0;
		}

		if (vm.count("version")) {
			std::cout << "mtfind Version 1.0" << std::endl;

			return 0;
		}

		if (vm.count("file")) {

			file_ = vm["file"].as<std::string>();

			boost::filesystem::path p = boost::filesystem::absolute(file_);

			bool exits = boost::filesystem::exists(p);

			if (!exits || (exits && boost::filesystem::is_directory(file_)))
			{
				std::cout << "Invalid file path specified." << std::endl;
				return -1;
			}
		}
		else
		{
			std::cout << "File path not specified." << std::endl;
			return -1;
		}

		if (vm.count("mask")) {

			mask_ = vm["mask"].as<std::string>();

			mask_.erase(std::remove(mask_.begin(), mask_.end(), '\"'), mask_.end());

			if (mask_.find_first_of('?') == std::string::npos ||
				mask_.find_first_of('\n') != std::string::npos )
			{
				std::cout << "Invalid mask specified." << std::endl;
				return -1;
			}

			if (mask_.size() > 100)
			{
				std::cout << "Too long mask." << std::endl;
				return -1;
			}
		}
		else
		{
			std::cout << "Mask to search not specified." << std::endl;
			return -1;
		}

		std::cout << "mtfind (c) 2022." << std::endl;
		std::cout << "file - " << file_ << " mask - " << mask_ << std::endl << std::endl;;

		SearchCore core_(file_, mask_);
		core_.Run();

		std::cout << "mtfind finished." << std::endl;

	}
	catch (std::exception& e)
	{
		std::cerr << e.what() << std::endl;
		return -1;
	}

	return 0;
}
