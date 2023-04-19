#include "reassembler_test_harness.hh"

#include <exception>
#include <iostream>

using namespace std;

int main()
{
  try {
    {
      ReassemblerTestHarness test { "all within capacity", 2 };

      test.execute( Insert { "ab", 0 } );
      test.execute( BytesPushed( 2 ) );
      test.execute( BytesPending( 0 ) );
      test.execute( ReadAll( "ab" ) );

      test.execute( Insert { "cd", 2 } );
      test.execute( BytesPushed( 4 ) );
      test.execute( BytesPending( 0 ) );
      test.execute( ReadAll( "cd" ) );

      test.execute( Insert { "ef", 4 } );
      test.execute( BytesPushed( 6 ) );
      test.execute( BytesPending( 0 ) );
      test.execute( ReadAll( "ef" ) );
    }

    {
      ReassemblerTestHarness test { "insert beyond capacity", 2 };

      test.execute( Insert { "ab", 0 } );
      test.execute( BytesPushed( 2 ) );
      test.execute( BytesPending( 0 ) );

      test.execute( Insert { "cd", 2 } );
      test.execute( BytesPushed( 2 ) );
      test.execute( BytesPending( 0 ) );

      test.execute( ReadAll( "ab" ) );
      test.execute( BytesPushed( 2 ) );
      test.execute( BytesPending( 0 ) );

      test.execute( Insert { "cd", 2 } );
      test.execute( BytesPushed( 4 ) );
      test.execute( BytesPending( 0 ) );

      test.execute( ReadAll( "cd" ) );
    }

    {
      ReassemblerTestHarness test { "overlapping inserts", 1 };

      test.execute( Insert { "ab", 0 } );
      test.execute( BytesPushed( 1 ) );
      test.execute( BytesPending( 0 ) );

      test.execute( Insert { "ab", 0 } );
      test.execute( BytesPushed( 1 ) );
      test.execute( BytesPending( 0 ) );

      test.execute( ReadAll( "a" ) );
      test.execute( BytesPushed( 1 ) );
      test.execute( BytesPending( 0 ) );

      test.execute( Insert { "abc", 0 } );
      test.execute( BytesPushed( 2 ) );
      test.execute( BytesPending( 0 ) );

      test.execute( ReadAll( "b" ) );
      test.execute( BytesPushed( 2 ) );
      test.execute( BytesPending( 0 ) );
    }

    {
      ReassemblerTestHarness test { "insert beyond capacity repeated with different data", 2 };

      test.execute( Insert { "b", 1 } );
      test.execute( BytesPushed( 0 ) );
      test.execute( BytesPending( 1 ) );

      test.execute( Insert { "bX", 2 } );
      test.execute( BytesPushed( 0 ) );
      test.execute( BytesPending( 1 ) );

      test.execute( Insert { "a", 0 } );

      test.execute( BytesPushed( 2 ) );
      test.execute( BytesPending( 0 ) );
      test.execute( ReadAll( "ab" ) );

      test.execute( Insert { "bc", 1 } );
      test.execute( BytesPushed( 3 ) );
      test.execute( BytesPending( 0 ) );

      test.execute( ReadAll( "c" ) );
    }
    {
      ReassemblerTestHarness test { "me1", 65000 };

      test.execute( Insert { "cdefg", 2 } );
      test.execute( BytesPushed( 0 ) );

      test.execute( Insert { "abcd", 0 } );
      test.execute( BytesPushed( 7 ) );
    }
    {
      ReassemblerTestHarness test { "me2", 20 };

      test.execute( Insert { "defg", 3 } );
      test.execute( BytesPushed( 0 ) );
      test.execute( BytesPending( 4 ) );

      test.execute( Insert { "defgh", 3 } );
      test.execute( BytesPushed( 0 ) );
      test.execute( BytesPending( 5 ) );

      test.execute( Insert { "defghi", 3 } );
      test.execute( BytesPushed( 0 ) );
      test.execute( BytesPending( 6 ) );

      test.execute( Insert { "cdefghi", 2 } );
      test.execute( BytesPushed( 0 ) );
      test.execute( BytesPending( 7 ) );

      test.execute( Insert { "cdefgh", 2 } );
      test.execute( BytesPushed( 0 ) );
      test.execute( BytesPending( 7 ) );

      test.execute( Insert { "bcdef", 1 } );
      test.execute( BytesPushed( 0 ) );
      test.execute( BytesPending( 8 ) );

      test.execute( Insert { "cdef", 2 } );
      test.execute( BytesPushed( 0 ) );
      test.execute( BytesPending( 8 ) );

      test.execute( Insert { "abc", 0 } );
      test.execute( BytesPushed( 9 ) );
      test.execute( BytesPending( 0 ) );

      test.execute( ReadAll( "abcdefghi" ) );
    }
    {
      ReassemblerTestHarness test { "me3", 20 };

      test.execute( Insert { "cd", 2 } );
      test.execute( BytesPushed( 0 ) );
      test.execute( BytesPending( 2 ) );

      test.execute( Insert { "fghijk", 6 } );
      test.execute( BytesPushed( 0 ) );
      test.execute( BytesPending( 8 ) );

      test.execute( Insert { "efgh", 5 } );
      test.execute( BytesPushed( 0 ) );
      test.execute( BytesPending( 9 ) );
    }

  } catch ( const exception& e ) {
    cerr << "Exception: " << e.what() << endl;
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
