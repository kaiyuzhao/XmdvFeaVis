### FeatureXmdv
Interactive feature exploration based on xmdvtool

### Project Description

A visual analytic workflow is implemented based on [XmdvTool](http://davis.wpi.edu/xmdv/) for modeling the feature space of input data to identify a subset of interesting features for further tasks. The project integrates both clustering model and user-driven selections into an interactive feature analytics pipeline where the user interact with a cluster model space of all the features. The model of all the features are created based on the redundancy relationship iof all the features which s established by using pairwise metrics such as information entropy and correlation coefficient.

#### Dependencies

1. Qt 4.8.1
2. R 2.15.1
3. Rtools 2.15
4. RInside 0.2.10

#### How to build

It is strongly recommended to compile/build all the dependencies from source code using your system default c++ compiler, otherwise there will be linkage errors.

##### Windows

1. Install [R 2.15.1](http://cran.us.r-project.org/) and set enviornment variable R_HOME to the installation directory
2. Install [Qt 4.8.1](https://download.qt.io/archive/qt/4.8/4.8.1/) to your system
3. Download [Rtools 2.15](https://cran.r-project.org/bin/windows/Rtools/) and install it, you may want to use the compiler comes with Rtools as your default compiler for this project. Then rebuild Qt libraries using it.
4. Download [RInside](http://dirk.eddelbuettel.com/code/rinside.html) and follow the [Embedding Qt Example](http://dirk.eddelbuettel.com/blog/2011/03/25#rinside_and_qt) to setup your Qt.
5. Buid this project using Qt Creater

#### Mac

1. Download [R 2.15.1](http://cran.us.r-project.org/) source code and compile it using your system compiler
2. Download [Qt 4.8.1](https://download.qt.io/archive/qt/4.8/4.8.1/) to your system
4. Download [RInside](http://dirk.eddelbuettel.com/code/rinside.html) and follow the [Embedding Qt Example](http://dirk.eddelbuettel.com/blog/2011/03/25#rinside_and_qt) to setup your Qt.
5. Buid the project
