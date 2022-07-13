#ifndef _WIN32
#include <unistd.h>
#endif

#include <iostream>

#include "QgisUntwine.hpp"

int main()
{
    untwine::QgisUntwine::StringList files;
    untwine::QgisUntwine::Options options;
    std::string exe = "C:\\Users\\andre\\untwine\\build\\untwine.exe";

    untwine::QgisUntwine api(exe);
    
    std::vector<unsigned char> funnycVec = { 0xc4, 0x8d };
    std::string funnyc(funnycVec.begin(), funnycVec.end());
    std::string v8string { "C:\\Users\\andre\\untwine\\api\\" + funnyc + "\\" + funnyc + ".las" };
    files.push_back(v8string);
    std::string outDir { "./out_" + funnyc };
    bool ok = api.start(files, outDir, options);
    if (! ok)
    {
        std::cerr << "Couldn't start '" << exe << "!\n";
        exit(-1);
    }

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
        if (!api.running())
            break;
    }
    std::cerr << "Error = " << api.errorMessage() << "\n";
}
