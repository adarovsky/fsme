/*
FSME - A State Machine Editor
Copyright (c) 2003-2005 Alexander Darovsky

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/
/*********************************************************************
 * Filename:      exception.h
 * 
 *                
 * Description:   A simple exception
 * Author:        Alexander Darovsky <adarovsky@sourceforge.net>
 *                
 * Created at:    Wed Nov 28 18:15:04 2001
 * Modified at:   Tue Feb 24 21:08:05 2004
 * Modified by:   Alexander Darovsky <adarovsky@sourceforge.net>
 *                
 * Version:       
 * Update count:  1
 *                
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 ********************************************************************/
#ifndef __TRANSLATOR_EXCEPTION__
#define __TRANSLATOR_EXCEPTION__
#include <qstring.h>

class Exception {
    QString m, s;
    int l;
protected:
    void setPlain( QString m_ ) { m = m_; }
public:
    Exception( QString m_, QString s_, int l_ );
    virtual QString message() const { return m; }
    virtual int     pos() const { return l; }
    virtual ~Exception() {}
};
#endif
