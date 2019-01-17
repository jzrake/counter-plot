#include "Runtime.hpp"
#include "DataHelpers.hpp"
#include "../Plotting/FigureView.hpp"




//=============================================================================
namespace builtin
{


    //=========================================================================
    template<typename T>
    T checkArg (const char* caller, var::NativeFunctionArgs args, int index)
    {
        if (index >= args.numArguments)
        {
            throw std::runtime_error (std::string (caller)
                                      + ": required argument at index "
                                      + std::to_string (index) + " not found");
        }
        return args.arguments[index];
    }

    template<>
    std::string checkArg<std::string> (const char* caller, var::NativeFunctionArgs args, int index)
    {
        if (index >= args.numArguments)
        {
            throw std::runtime_error (std::string (caller)
                                      + ": required argument at index "
                                      + std::to_string (index)
                                      + " not found");
        }
        return args.arguments[index].toString().toStdString();
    }

    template<>
    nd::array<double, 1> checkArg<nd::array<double, 1>> (const char* caller, var::NativeFunctionArgs args, int index)
    {
        if (index >= args.numArguments)
        {
            throw std::runtime_error (std::string (caller)
                                      + ": required argument at index "
                                      + std::to_string (index)
                                      + " not found");

        }

        if (auto arr = args.arguments[index].getArray())
        {
            nd::array<double, 1> result (arr->size());

            for (int n = 0; n < result.size(); ++n)
                result(n) = arr->getUnchecked(n);
            return result;
        }
        return Runtime::check_data<nd::array<double, 1>> (args.arguments[index]);
    }

    template<typename T>
    const T& checkArgData (const char* caller, var::NativeFunctionArgs args, int index)
    {
        if (index >= args.numArguments)
        {
            throw std::runtime_error (std::string (caller)
                                      + ": required argument at index "
                                      + std::to_string (index)
                                      + " not found");
        }
        try {
            return Runtime::check_data<T> (args.arguments[index]);
        }
        catch (const std::exception& e)
        {
            throw std::runtime_error (std::string (caller)
                                      + ": wrong type for argument at index "
                                      + std::to_string (index));
        }
    }

    template<typename T>
    T optKeywordArg (var::NativeFunctionArgs args, String key, T defaultValue)
    {
        return args.thisObject.getProperty (key, defaultValue);
    }


    //=========================================================================
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


    //=========================================================================
    var linspace (var::NativeFunctionArgs args)
    {
        auto x0  = checkArg<double> ("linspace", args, 0);
        auto x1  = checkArg<double> ("linspace", args, 1);
        auto num = checkArg<int>    ("linspace", args, 2);
        return Runtime::make_data (nd::linspace<double> (x0, x1, num));
    }


    //=========================================================================
    var plot (var::NativeFunctionArgs args)
    {
        LinePlotModel model;
        model.x.become (checkArg<nd::array<double, 1>> ("plot", args, 0));
        model.y.become (checkArg<nd::array<double, 1>> ("plot", args, 1));
        model.lineWidth        = optKeywordArg (args, "lw", 2.f);
        model.lineColour       = DataHelpers::colourFromVar (optKeywordArg (args, "lc", model.lineColour.toString()));
        model.markerSize       = optKeywordArg (args, "mw", model.markerSize);
        model.markerEdgeWidth  = optKeywordArg (args, "mew", model.markerEdgeWidth);
        model.markerEdgeColour = DataHelpers::colourFromVar (optKeywordArg<var> (args, "mec", model.markerEdgeColour.toString()));
        model.markerFillColour = DataHelpers::colourFromVar (optKeywordArg<var> (args, "mfc", model.markerFillColour.toString()));

        auto lineStyleString = optKeywordArg (args, "ls", String ("solid"));
        auto markStyleString = optKeywordArg (args, "ms", String ("none"));

        if (lineStyleString.isNotEmpty())
        {
            if (lineStyleString == "none")    model.lineStyle = LineStyle::none;
            if (lineStyleString == "solid")   model.lineStyle = LineStyle::solid;
            if (lineStyleString == "dash")    model.lineStyle = LineStyle::dash;
            if (lineStyleString == "dashdot") model.lineStyle = LineStyle::dashdot;
        }

        if (markStyleString.isNotEmpty())
        {
            if (markStyleString == "none")    model.markerStyle = MarkerStyle::none;
            if (markStyleString == "circle")  model.markerStyle = MarkerStyle::circle;
            if (markStyleString == "cross")   model.markerStyle = MarkerStyle::cross;
            if (markStyleString == "diamond") model.markerStyle = MarkerStyle::diamond;
            if (markStyleString == "plus")    model.markerStyle = MarkerStyle::plus;
            if (markStyleString == "square")  model.markerStyle = MarkerStyle::square;
        }

        if (model.x.size() != model.y.size())
        {
            throw std::runtime_error ("x and y have different sizes: "
                                      + std::to_string (model.x.size()) + " and "
                                      + std::to_string (model.y.size()));
        }
        return Runtime::make_data (std::shared_ptr<PlotArtist> (new LinePlotArtist (model)));
    }


    //=========================================================================
    var load_hdf5 (var::NativeFunctionArgs args)
    {
        auto _ = nd::axis::all();
        auto fname = checkArg<std::string> ("load-hdf5", args, 0);
        auto dname = checkArg<std::string> ("load-hdf5", args, 1);
        auto skip = optKeywordArg (args, "skip", 1);

        auto h5f = h5::File(fname, "r");
        auto arr = h5f.read<nd::array<double, 1>> (dname);
        return Runtime::make_data (arr.select (_|0|int(arr.size())|skip));
    }
}




// ============================================================================
void Runtime::load_builtins (Kernel& kernel)
{
    kernel.insert ("list", var::NativeFunction (builtin::list), Flags::builtin);
    kernel.insert ("dict", var::NativeFunction (builtin::dict), Flags::builtin);
    kernel.insert ("attr", var::NativeFunction (builtin::attr), Flags::builtin);
    kernel.insert ("item", var::NativeFunction (builtin::item), Flags::builtin);
    kernel.insert ("plot", var::NativeFunction (builtin::plot), Flags::builtin);
    kernel.insert ("linspace", var::NativeFunction (builtin::linspace), Flags::builtin);
    kernel.insert ("load-hdf5", var::NativeFunction (builtin::load_hdf5), Flags::builtin);
}
