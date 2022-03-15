#include <string>
#include <iostream>
#include <fstream>
#include "compile.h"
#include "checks.h"
#include "io.h"
#include "Lexer.h"
#include "Parser.h"
#include "ast/ModuleAST.h"
#include "Exception.h"

int compile(CompileArgs args)
{
    if (args.fnames.size() == 0)
    {
        std::cout << BasicException("no file name(s) specified").what() << '\n';
        return 1;
    }

    if (args.debug)
        // validate enums
        runChecks();

    for (auto fname : args.fnames)
    {
        std::ifstream fs(fname);
        if (!fs.is_open())
        {
            std::cout << BasicException(fmt::format("file '{}' was not found", fname)).what() << '\n';
            return 1;
        }
        std::string src = readFile(fs);
        Parser parser(fname, src);
        try {
            std::unique_ptr<ModuleAST> ast = parser.parse();
            if (args.debug)
                std::cout << stringify(ast.get());
            CodegenVisitor cg(fname, std::move(ast));
            cg.codegen();
            if (args.emit)
                cg.emitObjectCode();
            if (args.debug)
                cg.print();
        } catch (const BaseException& e) {
            std::cout << e.what() << '\n';
            return 1;
        }
    }

    return 0;
}
