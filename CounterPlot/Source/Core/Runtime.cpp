#include <fstream>
#include "Runtime.hpp"
#include "DataHelpers.hpp"
#include "AsciiLoader.hpp"
#include "../Plotting/Artists.hpp"




//=============================================================================
Identifier VarCallAdapter::BailoutChecker::argkey = "__bailout__";




//=============================================================================
namespace builtin
{
    void checkIndexValid (const char* caller, var::NativeFunctionArgs args, int index)
    {
        if (index >= args.numArguments)
        {
            throw std::runtime_error (std::string (caller)
                                      + ": required argument at index "
                                      + std::to_string (index) + " not found");
        }
    }

    //=========================================================================
    template<typename T=var>
    T checkArg (const char* caller, var::NativeFunctionArgs args, int index)
    {
        checkIndexValid (caller, args, index);
        return args.arguments[index];
    }

    template<>
    std::string checkArg<std::string> (const char* caller, var::NativeFunctionArgs args, int index)
    {
        return checkArg (caller, args, index).toString().toStdString();
    }

    template<>
    File checkArg<File> (const char* caller, var::NativeFunctionArgs args, int index)
    {
        auto path = checkArg<String> (caller, args, index);

        if (File::isAbsolutePath (path))
        {
            return File (path);
        }
        throw std::runtime_error ("must be an absolute path");
    }

    template<>
    nd::array<double, 1> checkArg<nd::array<double, 1>> (const char* caller, var::NativeFunctionArgs args, int index)
    {
        auto value = checkArg (caller, args, index);

        if (value.isArray())
            return DataHelpers::ndarrayDouble1FromVar (value);

        return Runtime::check_data<nd::array<double, 1>> (value);
    }

    template<>
    Array<var> checkArg<Array<var>> (const char* caller, var::NativeFunctionArgs args, int index)
    {
        auto value = checkArg (caller, args, index);

        if (value.isArray())
            return *value.getArray();

        throw Runtime::make_type_error (Runtime::type_name (Array<var>()), value, caller, index);
    }

    template<>
    var::NativeFunction checkArg<var::NativeFunction> (const char* caller, var::NativeFunctionArgs args, int index)
    {
        auto value = checkArg (caller, args, index);

        if (value.isMethod())
            return value.getNativeFunction();

        throw Runtime::make_type_error (Runtime::type_name (var::NativeFunction()), value, caller, index);
    }

    nd::array<double, 1> checkFlatten (const char* caller, var::NativeFunctionArgs args, int index)
    {
        auto value = checkArg (caller, args, index);

        if (auto result = dynamic_cast<Runtime::Data<nd::array<double, 1>>*> (value.getObject()))
            return result->value;
        if (auto result = dynamic_cast<Runtime::Data<nd::array<double, 2>>*> (value.getObject()))
            return result->value.reshape (int (result->value.size()));
        if (auto result = dynamic_cast<Runtime::Data<nd::array<double, 3>>*> (value.getObject()))
            return result->value.reshape (int (result->value.size()));

        return Runtime::check_data<nd::array<double, 1>> (value, caller, index); // will throw
    }

    template<typename T>
    const T& checkArgData (const char* caller, var::NativeFunctionArgs args, int index)
    {
        checkIndexValid (caller, args, index);
        return Runtime::check_data<T> (args.arguments[index], caller, index);
    }

    template<typename T>
    T optKeywordArg (var::NativeFunctionArgs args, String key, T defaultValue)
    {
        return args.thisObject.getProperty (key, defaultValue);
    }

    std::function<bool()> optBailout (var::NativeFunctionArgs args)
    {
        if (args.thisObject.hasProperty (VarCallAdapter::BailoutChecker::argkey))
        {
            auto value = args.thisObject[VarCallAdapter::BailoutChecker::argkey];
            return dynamic_cast<VarCallAdapter::BailoutChecker*> (value.getObject())->callback;
        }
        return nullptr;
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
        auto arr = checkArg<Array<var>> ("item", args, 0);
        auto ind = checkArg<int> ("item", args, 1);
        return arr[ind];
    }
    var attr (var::NativeFunctionArgs args)
    {
        return checkArg<var> ("attr", args, 0)[Identifier (checkArg<String> ("attr", args, 1))];
    }
    var len (var::NativeFunctionArgs args)
    {
        return args.numArguments >= 1 ? args.arguments[0].size() : 0;
    }


    //=========================================================================
    var map (var::NativeFunctionArgs args)
    {
        auto bailout = optBailout (args);
        auto op = checkArg<var::NativeFunction> ("map", args, 0);
        auto res = Array<var>();

        auto argsSlice = [args] (int argNumber) -> Array<var>
        {
            Array<var> result;

            for (int s = 1; s < args.numArguments; ++s)
            {
                if (auto arr = args.arguments[s].getArray())
                    result.add (argNumber < arr->size() ? arr->getUnchecked (argNumber) : var());
                else
                    result.add (args.arguments[s]);
            }
            return result;
        };

        auto selfSlice = [args] (int argNumber) -> var
        {
            auto result = std::make_unique<DynamicObject>();

            for (const auto& item : args.thisObject.getDynamicObject()->getProperties())
            {
                if (auto arr = item.value.getArray())
                    result->setProperty (item.name, argNumber < arr->size() ? arr->getUnchecked (argNumber) : var());
                else
                    result->setProperty (item.name, item.value);
            }
            return result.release();
        };

        int arrayArgLength = 0;

        for (int n = 1; n < args.numArguments; ++n)
        {
            if (args.arguments[n].size() > 0)
            {
                arrayArgLength = args.arguments[n].size();
                break;
            }
        }

        for (int n = 0; n < arrayArgLength; ++n)
        {
            if (bailout && bailout())
            {
                return var();
            }
            auto args = argsSlice(n);
            auto self = selfSlice(n);
            res.add (op (var::NativeFunctionArgs (self, args.begin(), args.size())));
        }
        return res;
    }


    //=========================================================================
    var join (var::NativeFunctionArgs args)
    {
        auto argsArray = Array<var> (args.arguments, args.numArguments);
        auto sep = optKeywordArg<String> (args, "sep", " ");
        return DataHelpers::stringArrayFromVar (argsArray).joinIntoString (sep);
    }

    var basename (var::NativeFunctionArgs args)
    {
        return checkArg<File> ("basename", args, 0).getFileName();
    }

    var format (var::NativeFunctionArgs args)
    {
        char result[256];
        auto format = checkArg<String> ("format", args, 0).toStdString();
        auto arg    = checkArg ("format", args, 1);
        if      (arg.isInt())    std::snprintf (result, 256, format.data(), int (arg));
        else if (arg.isDouble()) std::snprintf (result, 256, format.data(), double (arg));
        return String (result);
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
    var log10 (var::NativeFunctionArgs args)
    {
        auto value = checkArg ("log10", args, 0);

        if (value.isDouble())
        {
            return std::log10 (double (value));
        }
        if (auto A = Runtime::opt_data<nd::array<double, 1>> (value))
        {
            auto res = A->copy();
            for (auto& x : res) x = std::log10(x);
            return Runtime::make_data (res);
        }
        if (auto A = Runtime::opt_data<nd::array<double, 2>> (value))
        {
            auto res = A->copy();
            for (auto& x : res) x = std::log10(x);
            return Runtime::make_data (res);
        }
        if (auto A = Runtime::opt_data<nd::array<double, 3>> (value))
        {
            auto res = A->copy();
            for (auto& x : res) x = std::log10(x);
            return Runtime::make_data (res);
        }
        throw std::runtime_error ("log10 requires an array of type double");
    }


    //=========================================================================
    var min (var::NativeFunctionArgs args)
    {
        auto value = checkArg ("min", args, 0);
        if (value.isArray()) value = Runtime::make_data (DataHelpers::ndarrayDouble1FromVar (value));
        if (auto A = Runtime::opt_data<nd::array<double, 1>> (value)) return *std::min_element (A->begin(), A->end());
        if (auto A = Runtime::opt_data<nd::array<double, 2>> (value)) return *std::min_element (A->begin(), A->end());
        if (auto A = Runtime::opt_data<nd::array<double, 3>> (value)) return *std::min_element (A->begin(), A->end());
        throw std::runtime_error ("min requires an array of type double");
    }


    //=========================================================================
    var max (var::NativeFunctionArgs args)
    {
        auto value = checkArg ("max", args, 0);
        if (value.isArray()) value = Runtime::make_data (DataHelpers::ndarrayDouble1FromVar (value));
        if (auto A = Runtime::opt_data<nd::array<double, 1>> (value)) return *std::max_element (A->begin(), A->end());
        if (auto A = Runtime::opt_data<nd::array<double, 2>> (value)) return *std::max_element (A->begin(), A->end());
        if (auto A = Runtime::opt_data<nd::array<double, 3>> (value)) return *std::max_element (A->begin(), A->end());
        throw std::runtime_error ("max requires an array of type double");
    }


    //=========================================================================
    var cartprod (var::NativeFunctionArgs args)
    {
        auto bailout = optBailout (args);
        auto x = checkArg<nd::array<double, 1>> ("cartprod", args, 0);
        auto y = checkArg<nd::array<double, 1>> ("cartprod", args, 1);
        auto c = nd::array<double, 3> (x.size(), y.size(), 2);

        for (int i = 0; i < x.size(); ++i)
        {
            if (bailout && bailout())
                break;

            for (int j = 0; j < y.size(); ++j)
            {
                c(i, j, 0) = x(i);
                c(i, j, 1) = y(j);
            }
        }
        return Runtime::make_data(c);
    }


    //=========================================================================
    var sph_to_cart (var::NativeFunctionArgs args)
    {
        auto bailout = optBailout (args);
        auto a = checkArgData<nd::array<double, 3>> ("sph-to-cart", args, 0);
        auto b = nd::array<double, 3> (a.shape());

        for (int i = 0; i < a.shape(0); ++i)
        {
            for (int j = 0; j < a.shape(1); ++j)
            {
                const auto r = a(i, j, 0);
                const auto q = a(i, j, 1);
                const auto x = r * std::sin(q);
                const auto z = r * std::cos(q);
                b(i, j, 0) = x;
                b(i, j, 1) = z;
            }
            if (bailout && bailout())
            {
                break;
            }
        }
        return Runtime::make_data(b);
    }


    //=========================================================================
    var to_gpu_triangulate (var::NativeFunctionArgs args)
    {
        auto bailout = optBailout (args);
        auto vertices = checkArgData<nd::array<double, 3>> ("to_gpu_triangulate", args, 0);
        auto triverts = MeshHelpers::triangulateQuadMesh (vertices, bailout);

        if (bailout && bailout())
        {
            return var();
        }
        return Runtime::make_data (DeviceBufferFloat2 (triverts));
    }


    //=========================================================================
    var to_gpu (var::NativeFunctionArgs args)
    {
        auto data = checkFlatten  ("to_gpu_replicate", args, 0);
        auto reps = optKeywordArg<int> (args, "replicate", 1);

        std::vector<simd::float1> result;
        result.reserve (data.size() * reps);

        for (const auto& x : data)
            for (int n = 0; n < reps; ++n)
                result.push_back(x);

        return Runtime::make_data (DeviceBufferFloat1 (result));
    }


    //=========================================================================
    var scalar_mapping (var::NativeFunctionArgs args)
    {
        auto m  = ScalarMapping();
        m.vmin  = checkArg<float> ("scalar-mapping", args, 0);
        m.vmax  = checkArg<float> ("scalar-mapping", args, 1);
        m.stops = checkArgData<Array<Colour>> ("scalar-mapping", args, 2);
        return Runtime::make_data(m);
    }


    //=========================================================================
    var plot (var::NativeFunctionArgs args)
    {
        LinePlotModel model;
        model.x.become (checkArg<nd::array<double, 1>> ("plot", args, 0));
        model.y.become (checkArg<nd::array<double, 1>> ("plot", args, 1));
        model.lineWidth        = optKeywordArg (args, "lw", 2.f);
        model.markerSize       = optKeywordArg (args, "mw", model.markerSize);
        model.markerEdgeWidth  = optKeywordArg (args, "mew", model.markerEdgeWidth);
        model.lineColour       = DataHelpers::colourFromVar (optKeywordArg<var> (args, "lc",  model.lineColour.toString()));
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
    var trimesh (var::NativeFunctionArgs args)
    {
        auto vertices = checkArgData<DeviceBufferFloat2> ("trimesh", args, 0);
        auto scalars  = checkArgData<DeviceBufferFloat1> ("trimesh", args, 1);
        auto mapping  = checkArgData<ScalarMapping> ("trimesh", args, 2);
        auto trimesh  = std::make_shared<TriangleMeshArtist> (vertices, scalars, mapping);

        if (vertices.size != scalars.size)
        {
            throw std::runtime_error ("vertices and scalars have different sizes");
        }
        return Runtime::make_data (std::dynamic_pointer_cast<PlotArtist> (trimesh));
    }


    //=========================================================================
    var gradient (var::NativeFunctionArgs args)
    {
        auto stops       = checkArgData<Array<Colour>> ("gradient", args, 0);
        auto orientation = optKeywordArg<String> (args, "orientation", "vertical");
        auto transformed = optKeywordArg<bool>   (args, "transformed", false);
        auto gradient    = std::make_shared<ColourGradientArtist>();
        gradient->setStops (stops);
        gradient->setOrientation (orientation, true);
        gradient->setGradientFollowsTransform (transformed);
        return Runtime::make_data (std::dynamic_pointer_cast<PlotArtist> (gradient));
    }

    
    //=========================================================================
    var load_text (var::NativeFunctionArgs args)
    {
        auto fname = checkArg<std::string> ("load-text", args, 0);
        auto stream = std::ifstream (fname);
        auto loader = AsciiLoader (stream);

        if (! loader.getStatusMessage().empty())
        {
            throw std::runtime_error (loader.getStatusMessage());
        }

        auto columns = Array<var>();

        for (int n = 0; n < loader.getNumColumns(); ++n)
        {
            auto name = loader.getColumnName(n);
            auto data = nd::array<double, 1> (loader.getNumRows());
            loader.column (n, data.begin());
            columns.add (Runtime::make_data (data));
        }
        return columns;
    }


    //=========================================================================
    var load_hdf5 (var::NativeFunctionArgs args)
    {
        ScopedLock lock (DataHelpers::getCriticalSectionForHDF5());
        auto fname = checkArg<std::string> ("load-hdf5", args, 0);
        auto dname = checkArg<std::string> ("load-hdf5", args, 1);
        auto h5f = h5::File (fname, "r");
        auto h5d = h5f.open_dataset(dname);

        if (h5d.get_space().rank() == 0)
        {
            if (h5d.get_type() == h5::native_type<int>())
                return h5d.read<int>();
            if (h5d.get_type() == h5::native_type<double>())
                return h5d.read<double>();
            if (h5d.get_type() == h5::native_type<std::string>())
                return String (h5d.read<std::string>());
            throw std::runtime_error ("HDF5 unsupported scalar data type: " + dname);
        }
        if (h5d.get_space().rank() == 1)
        {
            auto _ = nd::axis::all();
            auto skip = optKeywordArg (args, "skip", 1);
            auto arr = h5f.read<nd::array<double, 1>> (dname);
            return Runtime::make_data (arr.select (_|0|int(arr.size())|skip));
        }
        if (h5d.get_space().rank() == 2)
        {
            auto arr = h5f.read<nd::array<double, 2>> (dname);
            return Runtime::make_data (arr);
        }
        throw std::runtime_error ("HDF5 dataset rank not 0, 1, or 2: " + dname);
    }


    //=========================================================================
    var load_patches2d (var::NativeFunctionArgs args)
    {
        auto _ = nd::axis::all();
        auto bailout = optBailout (args);
        auto file = checkArg<File> ("load-patches2d", args, 0);
        auto field = checkArg<std::string> ("load-patches2d", args, 1);
        auto component = optKeywordArg (args, "component", -1);
        auto fields = std::set<patches2d::Field> { patches2d::parse_field (field) };
        auto ser = FileSystemSerializer (file);
        auto db = patches2d::Database::load (ser, fields, bailout);
        auto res = Array<var>();

        for (auto patch : db)
        {
            if (component == -1)
                res.add (Runtime::make_data (patch.second));
            else if (component >= 0 && component < patch.second.shape(2))
                res.add (Runtime::make_data (patch.second.select (_, _, component).copy()));
            else
                throw std::runtime_error ("out-of-range component");
        }
        return res;
    }
}




// ============================================================================
void Runtime::load_builtins (Kernel& kernel)
{
    kernel.insert ("list",           var::NativeFunction (builtin::list),           Flags::builtin);
    kernel.insert ("dict",           var::NativeFunction (builtin::dict),           Flags::builtin);
    kernel.insert ("attr",           var::NativeFunction (builtin::attr),           Flags::builtin);
    kernel.insert ("item",           var::NativeFunction (builtin::item),           Flags::builtin);
    kernel.insert ("len",            var::NativeFunction (builtin::len),            Flags::builtin);
    kernel.insert ("map",            var::NativeFunction (builtin::map),            Flags::builtin);
    kernel.insert ("join",           var::NativeFunction (builtin::join),           Flags::builtin);
    kernel.insert ("basename",       var::NativeFunction (builtin::basename),       Flags::builtin);
    kernel.insert ("format",         var::NativeFunction (builtin::format),         Flags::builtin);
    kernel.insert ("log10",          var::NativeFunction (builtin::log10),          Flags::builtin);
    kernel.insert ("min",            var::NativeFunction (builtin::min),            Flags::builtin);
    kernel.insert ("max",            var::NativeFunction (builtin::max),            Flags::builtin);
    kernel.insert ("linspace",       var::NativeFunction (builtin::linspace),       Flags::builtin);
    kernel.insert ("cartprod",       var::NativeFunction (builtin::cartprod),       Flags::builtin);
    kernel.insert ("sph-to-cart",    var::NativeFunction (builtin::sph_to_cart),    Flags::builtin);
    kernel.insert ("scalar-mapping", var::NativeFunction (builtin::scalar_mapping), Flags::builtin);
    kernel.insert ("plot",           var::NativeFunction (builtin::plot),           Flags::builtin);
    kernel.insert ("trimesh",        var::NativeFunction (builtin::trimesh),        Flags::builtin);
    kernel.insert ("gradient",       var::NativeFunction (builtin::gradient),       Flags::builtin);
    kernel.insert ("load-text",      var::NativeFunction (builtin::load_text),      Flags::builtin);
    kernel.insert ("load-hdf5",      var::NativeFunction (builtin::load_hdf5),      Flags::builtin);
    kernel.insert ("load-patches2d", var::NativeFunction (builtin::load_patches2d), Flags::builtin);

    kernel.insert ("to-gpu-triangulate", var::NativeFunction (builtin::to_gpu_triangulate), Flags::builtin);
    kernel.insert ("to-gpu",             var::NativeFunction (builtin::to_gpu),             Flags::builtin);
}
