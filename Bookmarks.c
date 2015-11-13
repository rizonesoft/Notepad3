#ifdef BOOKMARK_EDITION
        POPUP "Bookmarks"
        BEGIN
            MENUITEM "Toggle\tCtrl+F2",             BME_EDIT_BOOKMARKTOGGLE
            MENUITEM SEPARATOR
            MENUITEM "Goto Next\tF2",               BME_EDIT_BOOKMARKNEXT
            MENUITEM "Goto Previous\tShift+F2",     BME_EDIT_BOOKMARKPREV
            MENUITEM SEPARATOR
            MENUITEM "Clear All\tAlt+F2",           BME_EDIT_BOOKMARKCLEAR
        END
#endif