#include <cstdlib>
#include <iostream>
#include <string>
#include <sys/wait.h>

/**
 * Executável ponte (target-runner) invocado pelo pacote irace.
 *
 * Argumentos recebidos pelo irace:
 *   argv[1]: <configuration_id>
 *   argv[2]: <instance_id>
 *   argv[3]: <seed>
 *   argv[4]: <instance_path>
 *   argv[5]: [bound] (opcional, caso maxTime/bound esteja ativo)
 *   argv[5..n]: parâmetros da configuração (--ri, --im, --ts, --i, --p)
 */
int main(int argc, char *argv[])
{
    if (argc < 5)
    {
        std::cerr << "Uso: target-runner <configuration_id> <instance_id> <seed> "
                  << "<instance> [bound] [parametros...]" << std::endl;
        return 1;
    }

    int primeiroParametro = 5;
    if (argc > 5 && std::string(argv[5]).rfind("--", 0) != 0)
    {
        primeiroParametro = 6;
    }

    std::string cmd = "./Main";
    for (int i = primeiroParametro; i < argc; i++)
    {
        cmd += " ";
        cmd += argv[i];
    }
    cmd += " < \"" + std::string(argv[4]) + "\"";

    int status = system(cmd.c_str());
    if (status == -1 || !WIFEXITED(status))
    {
        return 1;
    }
    return WEXITSTATUS(status);
}
