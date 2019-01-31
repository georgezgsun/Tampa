// --- Debugging --- 
// Flags for Debug Output
# define DEBUG_PRINT_FILEPATH
# define DEBUG_PRINT_SHORT_FN_NAME
# define DEBUG_PRINT_LINENUMBER

// Adjust Length of fields Here 
// FileName
# define DEBUG_FILEPATH_LENGTH 27
# define DEBUG_FILENAME_LENGTH 22
// Function
# define DEBUG_LONG_FUNCTION_LENGTH 40
# define DEBUG_SHORT_FUNCTION_LENGTH 25

// LineNumber
# define DEBUG_LINENUMBER_LENGTH 4

// file names
# if defined( DEBUG_PRINT_FILEPATH )
# define DEBUG_FILENAME ( QString( "%1").arg(__FILE__ , \
-DEBUG_FILEPATH_LENGTH, QLatin1Char(' ')) )

# elif defined( DEBUG_PRINT_FILENAME )
# include <QFileInfo>
# define DEBUG_FILENAME ( QString( "%1").arg( \
QFileInfo(__FILE__).fileName(), \
-DEBUG_FILENAME_LENGTH, QLatin1Char(' ')) )

# elif defined( DEBUG_NO_FILENAME )
# define DEBUG_FILENAME ""
# endif

// function Names
# if defined( Q_CC_GNU )
# if defined ( DEBUG_PRINT_LONG_FN_NAME )
# define DEBUG_FUNCTION_NAME QString( " %1").arg( __PRETTY_FUNCTION__, \
-DEBUG_FULL_FUNCTION_LENGTH, QLatin1Char(' '))
#
# elif defined ( DEBUG_PRINT_SHORT_FN_NAME )
# define DEBUG_FUNCTION_NAME QString( " %1").arg( __FUNCTION__ +QString("()"), \
-DEBUG_SHORT_FUNCTION_LENGTH, QLatin1Char(' '))
#
# elif defined ( DEBUG_NO_FUNCTION_NAME )
# define DEBUG_FUNCTION_NAME ""
# else
# error No debug fuction flags defined
# endif
# else
# define DEBUG_FUNCTION_NAME ""
# endif

// Line numbers
# if defined ( DEBUG_PRINT_LINENUMBER )
# define DEBUG_LINENUMBER ( QString("[%1]")\
.arg( QString::number(__LINE__), DEBUG_LINENUMBER_LENGTH, QLatin1Char(' ')) )
# else
# define DEBUG_LINENUMBER ""
# endif

// Debug Macro 
# define DEBUG() qDebug() << QString( DEBUG_FILENAME + DEBUG_LINENUMBER + DEBUG_FUNCTION_NAME + " :" )
