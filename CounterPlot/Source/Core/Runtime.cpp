#include "Runtime.hpp"
#include "../Plotting/FigureView.hpp"




// ============================================================================
namespace builtin
{
    template<typename T>
    T checkArg (var::NativeFunctionArgs args, int index)
    {
        if (index >= args.numArguments)
        {
            throw std::runtime_error ("required argument at index " + std::to_string (index) + " not found");
        }
        return args.arguments[index];
    }

    template<typename T>
    const T& checkArgData (var::NativeFunctionArgs args, int index)
    {
        if (index >= args.numArguments)
        {
            throw std::runtime_error ("required argument at index " + std::to_string (index) + " not found");
        }
        try {
            return Runtime::check_data<T> (args.arguments[index]);
        }
        catch (const std::exception& e)
        {
            DBG("checkArgData argument at index " << index);
            throw e;
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
        return checkArg<var>(args, 0)[checkArg<int>(args, 1)];
    }

    var attr (var::NativeFunctionArgs args)
    {
        return checkArg<var>(args, 0)[Identifier (checkArg<String>(args, 1))];
    }

    var linspace (var::NativeFunctionArgs args)
    {
        auto x0  = checkArg<double> (args, 0);
        auto x1  = checkArg<double> (args, 1);
        auto num = checkArg<int>    (args, 2);
        return Runtime::make_data (nd::linspace<double> (x0, x1, num));
    }

    var plot (var::NativeFunctionArgs args)
    {
        LinePlotModel model;
        model.x.become (checkArgData<nd::array<double, 1>> (args, 0));
        model.y.become (checkArgData<nd::array<double, 1>> (args, 1));
        return Runtime::make_data (std::shared_ptr<PlotArtist> (new LinePlotArtist (model)));
    }
}




// ============================================================================
void Runtime::load_builtins (Kernel& kernel)
{
    kernel.insert ("list", var::NativeFunction (builtin::list));
    kernel.insert ("dict", var::NativeFunction (builtin::dict));
    kernel.insert ("item", var::NativeFunction (builtin::item));
    kernel.insert ("linspace", var::NativeFunction (builtin::linspace));
    kernel.insert ("plot", var::NativeFunction (builtin::plot));
}
