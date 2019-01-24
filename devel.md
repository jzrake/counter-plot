# Counter Plot development log

## Jan 9, 2019: State and async tasks

I need to organize the way that data is loaded into the viewers, and how the viewers manage their state. This is sort of complex because (1) some of the work might be done asynchronously, (2) tasks sometimes need to be cancelled, (3) lightweight and heavyweight updates to the viewer state might be occuring at the same time. Whatever solution I decide on needs to be compatible with having extension viewers in Python or Lua.

Let's start with how things look from the perspective of a viewer. Its state can be described as:

(1) A file to be viewed
(2) UI data set by user interaction

In the simplest cases, (1) maps directly to a heavyweight data object, which should be loaded asynchronously. However, (2) might contain instructions for what should be loaded and displayed, so it could also trigger offline tasks to be launched.

Let's say that everything the viewer needs to display itself promptly is contained in a struct called State. Let's make State a read-only thing and use the redux language, where "reducers" are things that take an "action" and a previous state and produce a new state. Reducers can indicate whether they should be asynchronously or not.

But now we need to deal with what happens when a new action arrives before a reducer has fininshed its work. The simplest policy might be that new messages cancel any pending reducer tasks. But that is too crude. Consider if a new file is asked to be loaded, and in the mean time the user zooms the view. We don't want that to cancel the asynchronous file loading.

Let's then suppose that reducers cannot be asynchronous, but that they must return a new state promptly. Instead, we can have messages that are functions of another action and the current state. These functions can spend a while computing a new action offline - one which can be applied promptly to the state by a reducer. The action mutators are (I think) like redux middlewares, so let's call them that. Middlewares take a action, do some work offline, and then dispatch a new action to the store with the result of a calculation. Each middleware maintains its own slot in a scheduler - subsequent messages directed to the same middleware might cancel the current task.

UPDATE: This basically works. "Each slot in a scheduler" might be implemented as one thread pool per independent category of asynchronous task.


## Jan 10, 2019: Maintaining aspect during figure resize

It is sometimes desirable that resizing a figure component leaves the domain extent invariant, while other times the physical space represented in the figure should not be distorted. In other words, the domain extent is allowed to vary so as to leave e.g. the dimensions of a 1cm by 2cm ellipse unchanged.

The first case above is easy, because domain changes are always separate from plot area changes, whereas in the second case, both are changing simultaneously. It is important to determine whether this logic should be done in the figure or in the state management code. I implemented a first draft putting it in the figure component, which I still think is the right thing to do. To make it work, I needed to add a listener method that announces changes to the domain and the margin at the same time.


## Jan 13, 2019: Increasing codebase size

Recently I added YAML-configurable viewers. This has really clarified how the app should look after the next phase of development. User extension Views (term: Viewers, Viewers, Applets...?) will be defined by YAML documents. Each Viewer identifies possible files it is interested in, a set of figures, rules to lay them out, and a list of content for each figure. The Viewer also maintains an environment - a list of live variables that can be used in the figure contents list or as properties. The environment will contain (among other things) the name of the currently selected file in the directory tree. This means that the crt-kernel module I wrote previously will end up being utilized after all.

Anyway, as the code base begins growing it seems right to decide on terminology now, and also a sensible grouping of the source code files. I think the right name for FileBasedView is probably 'Viewer'. Code should be organized into these directories:

- Core
    + Main
    + Data
    + LookAndFeel
- Components
    + MainComponent
    + DirectoryTree
    + VariantView
- Plotting
    + MetalSurface
    + PlotModels
    + FigureView
- Viewers
    + ImageViewer
    + ColourMapViewer
    + JSONViewer
    + BinaryTorquesViewer
    + JetInCloudViewer
    + ExtensionViewer


## Jan 23, 2019: Async kernel resolves

I wrote a draft of the async kernel resolve strategy last weekend. However it needs to be improved. It cancels tasks unnecessarily:

1. x is enqueued
2. y is enqueued
3. x completes, triggers resolve
4. y is enqueued (thus cancelled) because it's still dirty

The simplest solution I could find was to unmark rules as soon as they are queued. This way they don't get cancelled and resubmitted unnecessarily. The side-effect is of course that rules downstream of an async rule may get started prematurely. The synchronous downstream rules are cheap anyway. And, in a busy situation, the asynchronous ones are likely to remain queued for a little bit, so they'll get superceded before they even consume any resources.

