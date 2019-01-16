#include "Runtime.hpp"
#include "../Plotting/FigureView.hpp"




// ============================================================================
namespace builtin
{
    template<typename T>
    T checkArg (const std::string& caller, var::NativeFunctionArgs args, int index)
    {
        if (index >= args.numArguments)
        {
            throw std::runtime_error (caller + ": required argument at index " + std::to_string (index) + " not found");
        }
        return args.arguments[index];
    }

    template<typename T>
    const T& checkArgData (const std::string& caller, var::NativeFunctionArgs args, int index)
    {
        if (index >= args.numArguments)
        {
            throw std::runtime_error (caller + ": required argument at index " + std::to_string (index) + " not found");
        }
        try {
            return Runtime::check_data<T> (args.arguments[index]);
        }
        catch (const std::exception& e)
        {
            throw std::runtime_error (caller + ": wrong type for argument at index " + std::to_string (index));
        }
    }

    var list (var::NativeFunctionArgs args)
    {
        return Array<var> (args.arguments, args.numArguments);
    }

    var dict (var::NativeFunctionArgs args)
    {
        return args.thisObject;
    }

    var item (var::NativeFunctionArgs args)
    {
        return checkArg<var>("item", args, 0)[checkArg<int>("item", args, 1)];
    }

    var attr (var::NativeFunctionArgs args)
    {
        return checkArg<var>("attr", args, 0)[Identifier (checkArg<String>("attr", args, 1))];
    }

    var linspace (var::NativeFunctionArgs args)
    {
        auto x0  = checkArg<double> ("linspace", args, 0);
        auto x1  = checkArg<double> ("linspace", args, 1);
        auto num = checkArg<int>    ("linspace", args, 2);
        return Runtime::make_data (nd::linspace<double> (x0, x1, num));
    }

    var plot (var::NativeFunctionArgs args)
    {
        LinePlotModel model;
        model.x.become (checkArgData<nd::array<double, 1>> ("plot", args, 0));
        model.y.become (checkArgData<nd::array<double, 1>> ("plot", args, 1));

        if (model.x.size() != model.y.size())
        {
            throw std::runtime_error ("x and y have different sizes: "
                                      + std::to_string (model.x.size()) + " and "
                                      + std::to_string (model.y.size()));
        }
        return Runtime::make_data (std::shared_ptr<PlotArtist> (new LinePlotArtist (model)));
    }
}




// ============================================================================
void Runtime::load_builtins (Kernel& kernel)
{
    kernel.insert ("list", var::NativeFunction (builtin::list), Flags::builtin);
    kernel.insert ("dict", var::NativeFunction (builtin::dict), Flags::builtin);
    kernel.insert ("item", var::NativeFunction (builtin::item), Flags::builtin);
    kernel.insert ("plot", var::NativeFunction (builtin::plot), Flags::builtin);
    kernel.insert ("linspace", var::NativeFunction (builtin::linspace), Flags::builtin);
}
