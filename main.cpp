﻿/*

	www.github.com/g40/esj

	The MIT License (MIT)

	Copyright (c) 2012-2014 Jerry Evans

	Permission is hereby granted, free of charge, to any person obtaining a copy
	of this software and associated documentation files (the "Software"), to deal
	in the Software without restriction, including without limitation the rights
	to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
	copies of the Software, and to permit persons to whom the Software is
	furnished to do so, subject to the following conditions:

	The above copyright notice and this permission notice shall be included in
	all copies or substantial portions of the Software.

	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
	AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
	OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
	THE SOFTWARE.


*/


#include <iostream>
#include <string>
#include <vector>
#include <stdio.h>

//-----------------------------------------------------------------------------
#include "json_writer.h"
#include "json_reader.h"
#include "pt.h"

//-----------------------------------------------------------------------------
// absolutely minimal example
class JSONExample
{
public:
	// content gets streamed to JSON
	std::string text;
public:
	void serialize(JSON::Adapter& adapter)
	{
		// this pattern is required 
		JSON::Class root(adapter,"JSONExample");
		// this is the last member variable we serialize so use the _T variant
		JSON_T(adapter,text);
	}
};

//-----------------------------------------------------------------------------
// test elementary template example
//
void test_templates()
{
	
	JSONExample source;
	source.text = "Hello World 2014";
	std::string json = JSON::producer<JSONExample>::convert(source);
	JSONExample sink = JSON::consumer<JSONExample>::convert(json);
	//
	JSON::throw_if(0,source.text != sink.text,"test_template failed");
}

//-----------------------------------------------------------------------------
// basic test rig for scanner
void test_scanner()
{
	Chordia::stringer json;
	//	sb << "{ \"text\" : \"\\u3086\\u304D\" , \"number\" : +123.456 }";
	// sb << "{ \"number\" : + }";
	json << "{"
			<< "\"tstate\" : true"
			<< "\"fstate\" : false"
			<< "\"nstate\" : null"
			<< "}";

	//
	JSON::StringSource source(json.c_str(),json.size());
	JSON::Scanner scanner(source);
	JSON::Reader reader(scanner);
	JSON::Token tt;
	while (tt.type != JSON::T_EOF)
	{
		tt = scanner.Scan();
		std::cout << "Token " << tt.type << " " << (tt.text.empty() ? "null" : tt.text) << std::endl;
	}
}


//-----------------------------------------------------------------------------
class InnerExample
{
	std::string text;

public:
	void serialize(JSON::Adapter& adapter)
	{
		// this pattern is required 
		JSON::Class root(adapter,"InnerExample");
		// this is the last member variable we serialize so use the _T variant
		JSON_T(adapter,text);
	}
	//
	bool operator==(const InnerExample& arg) const
	{
		return (text == arg.text);
	}
	// 
	InnerExample(const char* arg = 0) : text(arg?arg:"") {}
};

//-----------------------------------------------------------------------------
class OuterExample
{
public:

	//
	OuterExample() : m_inner("blah blah blah"), bool_data(false) {}

	// these items support serialization via JSON
	std::vector<std::string> string_data;
	std::vector<int> int_data;
	std::vector<double> double_data;
	std::vector<bool> v_bool_data;
	std::vector<InnerExample>  v_inner_data;
	InnerExample m_inner;
	bool bool_data;
	// check encoding round trips work
	std::wstring hiragana;
	//
	// 
	void serialize(JSON::Adapter& adapter)
	{
		// this pattern is required 
		JSON::Class root(adapter,"OuterExample");
		JSON_E(adapter,bool_data);
		JSON_E(adapter,string_data);
		JSON_E(adapter,int_data);
		JSON_E(adapter,double_data);
		JSON_E(adapter,v_inner_data);
		JSON_E(adapter,v_bool_data);
		JSON_E(adapter,hiragana);
		JSON_T(adapter,m_inner);
	}

	// for testing
	bool operator==(const OuterExample& arg) const
	{
		bool ret = (string_data == arg.string_data);
		ret &= (int_data == arg.int_data);
		// explicitly ignored. 
		// conversions from double to string render test unreliable
		// ret &= (double_data == arg.double_data);
		ret &= (bool_data == arg.bool_data);
		ret &= (v_inner_data == arg.v_inner_data);
		ret &= (v_bool_data == arg.v_bool_data);
		return ret;
	}
};

//-----------------------------------------------------------------------------
// elementary serialize => de-serialize example and unit test
void test_nesting()
{
	Chordia::stringer sb;

	bool ok = false;
	OuterExample tester;
	OuterExample tester2;

	// 'yuki'. relies on correct Windows code pages
	// so is not terribly portable ...
    
	//tester.hiragana = L"ゆき";

	tester.bool_data = false;
	tester.string_data.push_back("Hello");
	tester.string_data.push_back("world");
	tester.string_data.push_back("...");
	tester.int_data.push_back(1);
	tester.int_data.push_back(11);
	tester.int_data.push_back(111);
	tester.double_data.push_back(1.234);
	tester.double_data.push_back(11.234);
	tester.double_data.push_back(111.234);
	tester.v_bool_data.push_back(true);
	tester.v_bool_data.push_back(false);
	tester.v_bool_data.push_back(true);

	// a vector of InnerExample
	tester.v_inner_data.push_back(InnerExample("hello from Inner 0"));
	tester.v_inner_data.push_back(InnerExample("And hello from InnerExample 1"));

	Chordia::stringer sg;
	JSON::StringSink ssi(&sg);
	JSON::Writer writer(&ssi);
	tester.serialize(writer);
	DBMSG(sg.c_str());

	// now thaw the object from its JSON state
	JSON::StringSource sso(sg.c_str(),sg.size());
	JSON::Scanner scanner(sso);
	JSON::Reader reader(scanner);
	//
	tester2.serialize(reader);
	//
	ok = (tester == tester2);
	//
	JSON::throw_if(0,ok == false,"failure in test equality");
};

//-----------------------------------------------------------------------------
// check some of the numeric conversion routines
void test_conversion()
{
	//
	int iv = 12345;
	std::string siv = Chordia::toString(iv);
	// now convert to binary
	siv = Chordia::toString(iv,10);
	JSON::throw_if(0,siv != "12345","siv is not equal to 12345");

	//
	//double dv = 123123123123123.45678;
	//double dv = 123.45678;
	double dv = 1.234;
	// precision is 4 digits, rounded down by default
	std::string sdv = Chordia::DoubleConverter<>::Convert(dv);
	//
	Chordia::stringer sb;
	sb << sdv << " != 1.234";
	JSON::throw_if(0,sdv != "1.234",sb.c_str());

}

//-----------------------------------------------------------------------------
// test fromDouble conversion rates against 
void test_speed()
{
	int iv = 0;
	std::string sdv;
	const int maxiters = 100000;
	double dv = 1.2345;
	// test conversion speeds
	{
		Chordia::CPTSnap pts;
		for (int i = 0; i < maxiters; i++)
		{
			sdv = Chordia::DoubleConverter<>::Convert(dv);
		}
		Chordia::CPTInterval pti(pts);
		std::cout << sdv << " fromDouble " << pti.ms() << "ms" << std::endl;
	}

#ifdef _MSC_VER
	// test conversion speeds
	{
		Chordia::CPTSnap pts;
		for (int i = 0; i < maxiters; i++)
		{
			sdv = Chordia::toString(dv);
		}
		Chordia::CPTInterval pti(pts);
		std::cout << sdv << " toString " << pti.ms() << "ms" << std::endl;
	}
#endif

	{
		const int buffersize = 32;
		char buffer[buffersize];
		Chordia::CPTSnap pts;
		for (int i = 0; i < maxiters; i++)
		{
#ifdef _MSC_VER
			sprintf_s(buffer,buffersize,"%g",dv);
#else
			sprintf(buffer,"%g",dv);
#endif // _MSC_VER
			sdv = buffer;
		}
		Chordia::CPTInterval pti(pts);
		std::cout << sdv << " sprintf_s " << pti.ms() << "ms" << std::endl;
	}

	// integer conversions
	iv = 123;
	// test conversion speeds
	{
		Chordia::CPTSnap pts;
		for (int i = 0; i < maxiters; i++)
		{
			sdv = Chordia::toString(iv);
		}
		Chordia::CPTInterval pti(pts);
		std::cout << sdv << " toString " << pti.ms() << "ms" << std::endl;
	}

	{
		const int buffersize = 32;
		char buffer[buffersize];
		Chordia::CPTSnap pts;
		for (int i = 0; i < maxiters; i++)
		{
#ifdef _MSC_VER
			sprintf_s(buffer,buffersize,"%d",iv);
#else
			sprintf(buffer,"%d",iv);
#endif // _MSC_VER
			sdv = buffer;
		}
		Chordia::CPTInterval pti(pts);
		std::cout << sdv << " sprintf_s " << pti.ms() << "ms" << std::endl;
	}


	int_size imin = std::numeric_limits<int_size>::min(); // minimum value
	int_size imax = std::numeric_limits<int_size>::max();
	//
	std::cout << "Iteration count was " << maxiters << " +ve int64_t: " << imin << " -ve int64_t: " << imax << std::endl;

}

//-----------------------------------------------------------------------------
// Test conversions
void test_converter()
{
	std::string sdv;

	double dv = 1.23;
	// precision is 4 digits, rounded down by default
	sdv = Chordia::DoubleConverter<>::Convert(dv);
	JSON::throw_if(sdv != "1.23",Chordia::stringer() << sdv << "!= 1.23");

	// 
	dv = 0;
	sdv = Chordia::DoubleConverter<>::Convert(dv);
	JSON::throw_if(sdv != "0",Chordia::stringer() << sdv << "!= 0");

	dv = 1;
	sdv = Chordia::DoubleConverter<>::Convert(dv);
	JSON::throw_if(sdv != "1",Chordia::stringer() << sdv << "!= 1");

	dv = 1.23456;
	sdv = Chordia::DoubleConverter<>::Convert(dv);
	JSON::throw_if(sdv != "1.23456",Chordia::stringer() << sdv << "!= 1.23456");

	dv = -1.23456;
	sdv = Chordia::DoubleConverter<>::Convert(dv);
	JSON::throw_if(sdv != "-1.23456",Chordia::stringer() << sdv << "!= -1.23456");

	dv = 0.4999;
	sdv = Chordia::DoubleConverter<>::Convert(dv);
	JSON::throw_if(sdv != "0.4999",Chordia::stringer() << sdv << "!= 0.4999");

	dv = 0.498765;
	sdv = Chordia::DoubleConverter<>::Convert(dv);
	JSON::throw_if(sdv != "0.498765",Chordia::stringer() << sdv << "!= 0.498765");

	dv = -0.498765;
	sdv = Chordia::DoubleConverter<>::Convert(dv);
	JSON::throw_if(sdv != "-0.498765",Chordia::stringer() << sdv << "!= -0.498765");

	dv = 0.1;
	sdv = Chordia::DoubleConverter<>::Convert(dv);

	dv = 0.001;
	sdv = Chordia::DoubleConverter<>::Convert(dv);

	dv = 0.0000001;
	sdv = Chordia::DoubleConverter<>::Convert(dv,0.00000001);
	JSON::throw_if(sdv != "0.0000001",Chordia::stringer() << sdv << "!= 0.0000001");

	dv = 123.456;
	sdv = Chordia::DoubleConverter<>::Convert(dv);
	JSON::throw_if(sdv != "123.456",Chordia::stringer() << sdv << "!= 123.456");
}

//-----------------------------------------------------------------------------
// to nearest       FE_TONEAREST   _RC_NEAR
// toward zero      FE_TOWARDZERO  _RC_CHOP
// to +infinity     FE_UPWARD      _RC_UP
// to -infinity     FE_DOWNWARD    _RC_DOWN
//-----------------------------------------------------------------------------
// Runs a set of uni8t tests for various components ...
int main(int argc, char* argv[])
{
	// typedef function type for each test
	typedef void (*pfnTest)();
	// a vector of the same
	std::vector<pfnTest> tests;
	// populate the vector
	tests.push_back(test_speed);
	tests.push_back(test_converter);
	tests.push_back(test_conversion);
	tests.push_back(test_scanner);
	tests.push_back(test_templates);
	tests.push_back(test_nesting);

	// invoke each test - keep going even if we get failures
	int failures = 0;
	typedef std::vector<pfnTest>::iterator test_iterator;
	for (test_iterator tit = tests.begin(); tit != tests.end(); ++tit)
	{
		// exceptions disabled for GCC/ARMCC by default
#if defined(__arm__) || defined(__CLANG__)
		(*tit)();
#else
		try
		{
			(*tit)();
		}
		catch (std::exception& e)
		{
			std::cout << e.what() << std::endl;	
			failures++;
		}
		catch (...)
		{
			std::cout << "Unknown exception" << std::endl;		
			failures++;
		}
#endif		
	}

	//
	std::cout << "Unit tests: " << (failures == 0 ? "*PASS* " : "*FAIL* " ) << failures << " failures" << std::endl;
	//
	return (-failures);
}

//-----------------------------------------------------------------------------
// Placeholder to ensure clean linking for ARM builds.
#ifdef __arm__
extern "C"{

void SystemInit()
{

}

}
#endif
