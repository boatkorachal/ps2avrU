
#include "dualaction.h"
#include "keymap.h"
// #include "print.h"
#include "macrobuffer.h"
#include "quickmacro.h"
#include "keymatrix.h"
#include "keyindex.h"
#include "keyscan.h"
#include "hardwareinfo.h"
#include "oddebug.h"

static uint8_t dualActionKeyIndex = 0;
//static uint8_t _dualActionCount = 0;
static bool _isCompounded = false;
//static bool _isActiveDualAction = false;
static uint16_t _autoDownCount = 0;
static bool _alreadyActioned = false;

static uint8_t getDualActionCompoundKey(uint8_t keyidx);
static uint8_t getDualActionAloneKey(uint8_t keyidx);

static bool isCompounded(void)
{
	return _isCompounded;
}

void enterFrameForDualAction(void){
	/*
	 * If a dual action key is pressed and held for a certain period of time (about 0.5 second?), It is applied as a combination key
	 */
//	if(dualActionKeyIndex > 0 && ++_autoDownCount > 500){
	if(dualActionKeyIndex > 0 && _isCompounded == false && ++_autoDownCount > 500){
		_isCompounded = true;
		applyDualActionDownWhenIsCompounded();
	}

}

void applyDualActionDownWhenIsCompounded(void){

	if(dualActionKeyIndex > 0 && isCompounded() && false == _alreadyActioned){
        // When combined with other keys, first, the combination key code value of the dual action key is stored in the buffer.
        pushKeyCodeDecorator(getDualActionCompoundKey(dualActionKeyIndex), true);

        if(isQuickMacro()){
        	putKeyindex(getDualActionCompoundKey(dualActionKeyIndex), 0, 0, 1);
        }

//        dualActionKeyIndex = 0;
		_autoDownCount = 0;
		_alreadyActioned = true;
    }
}

static void applyDualActionUp(void){

    if(dualActionKeyIndex > 0 && !isCompounded()){
        // If a dual action is saved, and no key is pressed, the action will take place!
       	uint8_t gUpIdx = getDualActionAloneKey(dualActionKeyIndex);
        pushMacroKeyIndex(gUpIdx);
        pushMacroKeyIndex(gUpIdx);

        dualActionKeyIndex = 0;
		_autoDownCount = 0;

        if(isQuickMacro()){
        	putKeyindex(gUpIdx, 0, 0, 1);
        	putKeyindex(gUpIdx, 0, 0, 0);
        }
    }

    // 듀얼액션 키가 모두 up 되었을 때만 active 해제;
	/*if(_dualActionCount == 0) {
        _isActiveDualAction = false;

    }*/
}

void setDualAction(uint8_t keyidx, bool isDown){
//    DBG1(0xF0, (uchar *)&isDown, 1);

    /*
     * The first key (except modi) is activated by pressing the dual action key;
     *
     *
     */

	if(isDown){
//	    DBG1(0xF1, (uchar *)&keyidx, 1);
	    IF_IS_DUAL_ACTION_KEY(keyidx) {
			if (isReleaseAll() && dualActionKeyIndex == 0) { // The first key is pressed with a dual action key;

//			    DBG1(0xF1, (uchar *)&dualActionKeyIndex, 5);
				dualActionKeyIndex = keyidx;
				_isCompounded = false;  // Since only one has been pressed, it is not yet in combination;
			} else if (!isReleaseAll() && dualActionKeyIndex == 0) {
                dualActionKeyIndex = keyidx;
                _isCompounded = false;
            } else {

                // Two or more dual action keys pressed;
                /*
                * Only the first dual action key will act as a compound key,
                * The second keys act as default keys.
                */
				_isCompounded = true;
			}
		} else {

            /* If a key other than the dual action key is pressed first,
            *
            * 1. When the alone key is activated;
            * 2. When the compound key is activated
            */
			_isCompounded = true;
		}
//        DBG1(0xF2, (uchar *)&_isActiveDualAction, 2);
//        DBG1(0xF2, (uchar *)&_isCompounded, 3);
	} else {
//	    DBG1(0xF3, (uchar *)&keyidx, 1);
	    IF_IS_DUAL_ACTION_KEY(keyidx) {
//			if (_dualActionCount > 0) --_dualActionCount;

			applyDualActionUp();
		} else {

		}

	}
//    DBG1(0xF4, (uchar *)&_dualActionCount, 1);
}
void clearDualAction(void)
{
//    _dualActionCount = 0;
    dualActionKeyIndex = 0;
    _isCompounded = false;
    _alreadyActioned = false;
//    _isActiveDualAction = false;
}
//-------------------------------------------------------------------------------

uint8_t getDualActionDefaultKey(uint8_t xActionIndex){
    IF_IS_DUAL_ACTION_KEY(xActionIndex){
        // 조합 키가 우선이되 FN 계열의 키는 alone키로 반환
        uint8_t gIndex =  getDualActionCompoundKey(xActionIndex);
        if(isFnKey(gIndex)){
            return getDualActionAloneKey(xActionIndex);
        }
        return gIndex;
    }
    return xActionIndex;
}

// When the keys are combined, a combination key code is applied.;
uint8_t getDualActionDownKeyIndexWhenIsCompounded(uint8_t xKeyidx, bool xForceCompounded){
//    DBG1(0xF8, (uchar *)&xKeyidx, 2);
//    DBG1(0xF1, (uchar *)&dualActionKeyIndex, 3);
    IF_IS_DUAL_ACTION_KEY(xKeyidx){
        if(xForceCompounded == true)
        {
            return getDualActionCompoundKey(xKeyidx);
        }
        else if(isCompounded())
        {
            // 처음 눌린 키만 compound
            if(dualActionKeyIndex == xKeyidx)
            {
                return getDualActionCompoundKey(xKeyidx);
            }
            else
            {
                // 나머지는 default key
                return getDualActionDefaultKey(xKeyidx);
            }
        }
    }

//    DBG1(0xF8, (uchar *)&xKeyidx, 1);

    return xKeyidx;
}

// 조합 키코드를 반환한다.
static uint8_t getDualActionCompoundKey(uint8_t xActionIndex){
//    if(xActionIndex > KEY_dualAction && xActionIndex < KEY_dualAction_end){
        xActionIndex = getExchangedKeyindex(pgm_read_byte(DUALACTION_ADDRESS + (xActionIndex - (KEY_dualAction + 1)) * 2));
//    }
    return xActionIndex;
}

// 각개 키코드 반환;
static uint8_t getDualActionAloneKey(uint8_t xActionIndex){
//    if(xActionIndex > KEY_dualAction && xActionIndex < KEY_dualAction_end){
        xActionIndex = getExchangedKeyindex(pgm_read_byte(DUALACTION_ADDRESS + 1 + (xActionIndex - (KEY_dualAction + 1)) * 2));
//    }
    return xActionIndex;
}
