#include <StandaloneApplication.hpp>

int main(int argc, char** argv)
{
    cp::StandaloneApplication application;
    if (!application.Init(argc, argv))
    {
        application.Clear();
        return 1;
    }

    application.Run();
    application.Clear();
    return 0;
}
