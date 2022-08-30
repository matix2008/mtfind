#pragma once

#include <string>

#include <boost/filesystem.hpp>
#include "boost/filesystem/fstream.hpp"
#include <boost/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/regex.hpp>

class SearchCore
{
private:

	std::string _file, _mask;	// file path and mask to search
	std::vector<std::pair<int,std::string>> _input_storage; // lines storage
	std::vector<std::pair<std::pair<int,int>, std::string>> _result_storage; // results

	boost::mutex _mx_input;	// mutex for lines storage

	boost::mutex _mx;		// mutex for other
	int _ln_count;
	bool _no_data;

protected:

	// *************************************
	// split file to lines and insert it to storage
	// *************************************
	void load_lines()
	{
		boost::filesystem::ifstream fh_(_file);

		std::string line_;
		int lines = 0;

		while (std::getline(fh_, line_)) 
		{

			boost::mutex::scoped_lock lock(_mx_input);

			_input_storage.push_back(std::make_pair(++lines,line_));

			++_ln_count;
		}

		_no_data = true;
	}

	// *************************************
	// trying find results by mask specified
	// *************************************
	void search_result()
	{
		while (true)
		{
			{
				boost::mutex::scoped_lock lock(_mx_input);

				for (std::vector<std::pair<int, std::string>>::iterator it_ = _input_storage.begin();
					it_ != _input_storage.end(); ++it_)
				{
					boost::regex rx_(_mask);

					boost::smatch what;
					if (boost::regex_search((*it_).second, what, rx_))
					{
						_result_storage.push_back(
							std::make_pair(
								std::make_pair((*it_).first, // line number
									what.position()), // matched position
								what[0]) // matched string
						);
					}

					_ln_count--;
				}

				_input_storage.clear();
			}

			{
				boost::mutex::scoped_lock lock(_mx);
				if (_no_data && _ln_count == 0)
					return;
			}
		}
	}

	// *************************************
	// print results to console
	// *************************************
	void print_results()
	{
		std::cout << _result_storage.size() << std::endl;

		for (std::vector<std::pair<std::pair<int, int>, std::string>>::iterator it_ = _result_storage.begin();
			it_ != _result_storage.end(); ++it_)
		{
			std::cout	<< (*it_).first.first << " " 
						<< (*it_).first.second << " " 
						<< (*it_).second << std::endl << std::endl;
		}
	}

public:

	// *************************************
	// Construct the SearchCore object
	// *************************************
	SearchCore(std::string file, std::string mask)
	{
		_file = file;
		_mask = mask;

		// replace '?' to dot
		std::replace(_mask.begin(), _mask.end(), '?', '.');

		_no_data = false;
		_ln_count = 0;
	}

	// *************************************
	// main loop
	// *************************************
	void Run()
	{
		boost::thread input_thread(boost::bind(&SearchCore::load_lines, this));
		boost::thread search_thread(boost::bind(&SearchCore::search_result, this));

		input_thread.join();
		search_thread.join();

		print_results();
	}
};

