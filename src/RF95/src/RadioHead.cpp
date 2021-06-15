/* RadioHead library by William Espinoza
 */

#include "RF95/src/RadioHead.h"

/**
 * Constructor.
 */
RadioHead::RadioHead()
{
  // be sure not to call anything that requires hardware be initialized here, put those in begin()
}

/**
 * Example method.
 */
void RadioHead::begin()
{
    // initialize hardware
    Serial.println("called begin");
}

/**
 * Example method.
 */
void RadioHead::process()
{
    // do something useful
    Serial.println("called process");
    doit();
}

/**
* Example private method
*/
void RadioHead::doit()
{
    Serial.println("called doit");
}
