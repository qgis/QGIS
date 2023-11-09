#ifndef _WIN32
#include <unistd.h>
#endif

#include <iostream>

#include "QgisUntwine.hpp"

int main()
{
    untwine::QgisUntwine::StringList files;
    untwine::QgisUntwine::Options options;
//    std::string exe = "C:\\Users\\andre\\untwine\\build\\untwine.exe";
    std::string exe = "/Users/acbell/untwine/build/untwine";

    untwine::QgisUntwine api(exe);
    
//    files.push_back("C:\\Users\\andre\\nyc2");
//    std::vector<unsigned char> funnycVec = { 0xc4, 0x8d };
//    std::string funnyc(funnycVec.begin(), funnycVec.end());
//    std::string v8string { "C:\\Users\\andre\\untwine\\api\\build\\" + funnyc + "\\" + funnyc + ".las" };
//    files.push_back(v8string);
//   files.push_back("C:\\Users\\andre\\nyc2\\18TXL075075.las.laz");
//    files.push_back("/Users/acbell/nyc/18TXL075075.las.laz");
//    files.push_back("/Users/acbell/nyc/18TXL075090.las.laz");
//    files.push_back("/Users/acbell/nyc2");
    files.push_back("/Users/acbell/USGS_LPC_MD_PA_SandySupp_2014_18SUH825795_LAS_2016.laz");

//    options.push_back({"dims", "X, Y, Z, Red, Green, Blue, Intensity"});
//    book ok = api.start(files, ".\\out", options);
    bool ok = api.start(files, "./out", options);
    if (! ok)
    {
        std::cerr << "Couldn't start '" << exe << "!\n";
        exit(-1);
    }

    bool stopped = false;
    while (true)
    {
#ifdef _WIN32
    	Sleep(1000);
#else
        ::sleep(1);
#endif
        int percent = api.progressPercent();
        std::string s = api.progressMessage();
        std::cerr << "Percent/Msg = " << percent << " / " << s << "!\n";
        /**
        if (!stopped && percent >= 50)
        {
            stopped = true;
            api.stop();
        }
        **/
        if (!api.running())
            break;
    }
    std::cerr << "Error = " << api.errorMessage() << "\n";
}
