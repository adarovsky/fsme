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

#include "semantic.h"

#define TYPE(type) ParType(ParType::type, this)
#define LTYPE(type) ParType(ParType::type, this, ParType::LValue)
void Semantic::initFunctions()
{
//     funcList.addFunc( FuncDef( TYPE(Int),    "cast@double@int", TYPE(Double) ) );
    
//     funcList.addFunc( FuncDef( TYPE(Int),    "@=@",LTYPE(Int)   , TYPE(Int)    ) );
//     funcList.addFunc( FuncDef( TYPE(Double), "@=@",LTYPE(Double), TYPE(Double) ) );

//     funcList.addFunc( FuncDef( TYPE(Int),    "@+@", TYPE(Int)   , TYPE(Int)    ) );
//     funcList.addFunc( FuncDef( TYPE(Double), "@+@", TYPE(Double), TYPE(Double) ) );

//     funcList.addFunc( FuncDef( TYPE(Int),    "@-@", TYPE(Int)   , TYPE(Int)    ) );
//     funcList.addFunc( FuncDef( TYPE(Double), "@-@", TYPE(Double), TYPE(Double) ) );

//     funcList.addFunc( FuncDef( TYPE(Int),    "@*@", TYPE(Int)   , TYPE(Int)    ) );
//     funcList.addFunc( FuncDef( TYPE(Double), "@*@", TYPE(Double), TYPE(Double) ) );

//     funcList.addFunc( FuncDef( TYPE(Int),    "@/@", TYPE(Int)   , TYPE(Int)    ) );
//     funcList.addFunc( FuncDef( TYPE(Double), "@/@", TYPE(Double), TYPE(Double) ) );

//     funcList.addFunc( FuncDef( TYPE(Int),    "@&@", TYPE(Int)   , TYPE(Int)    ) );
//     funcList.addFunc( FuncDef( TYPE(Int),    "@|@", TYPE(Int)   , TYPE(Int)    ) );
//     funcList.addFunc( FuncDef( TYPE(Int),    "@^@", TYPE(Int)   , TYPE(Int)    ) );
//     funcList.addFunc( FuncDef( TYPE(Int),    "@%@", TYPE(Int)   , TYPE(Int)    ) );


//     funcList.addFunc( FuncDef( TYPE(Int),    "@<@", TYPE(Int)   , TYPE(Int)    ) );
//     funcList.addFunc( FuncDef( TYPE(Double), "@<@", TYPE(Double), TYPE(Double) ) );

//     funcList.addFunc( FuncDef( TYPE(Int),    "@>@", TYPE(Int)   , TYPE(Int)    ) );
//     funcList.addFunc( FuncDef( TYPE(Double), "@>@", TYPE(Double), TYPE(Double) ) );

//     funcList.addFunc( FuncDef( TYPE(Boolean),"@==@", TYPE(Boolean),TYPE(Boolean)) );
//     funcList.addFunc( FuncDef( TYPE(Int),    "@==@", TYPE(Int)   , TYPE(Int)    ) );
//     funcList.addFunc( FuncDef( TYPE(Double), "@==@", TYPE(Double), TYPE(Double) ) );

//     funcList.addFunc( FuncDef( TYPE(Boolean),"@!=@", TYPE(Boolean),TYPE(Boolean)) );
//     funcList.addFunc( FuncDef( TYPE(Int),    "@!=@", TYPE(Int)   , TYPE(Int)    ) );
//     funcList.addFunc( FuncDef( TYPE(Double), "@!=@", TYPE(Double), TYPE(Double) ) );

    funcList.addFunc( FuncDef( TYPE(Boolean),"@||@", TYPE(Boolean),TYPE(Boolean)) );
    funcList.addFunc( FuncDef( TYPE(Boolean),"@&&@", TYPE(Boolean),TYPE(Boolean)) );
    funcList.addFunc( FuncDef( TYPE(Boolean), "!@",   TYPE(Boolean)) );

//     funcList.addFunc( FuncDef( TYPE(Int),    "@>=@", TYPE(Int)   , TYPE(Int)    ) );
//     funcList.addFunc( FuncDef( TYPE(Double), "@>=@", TYPE(Double), TYPE(Double) ) );

//     funcList.addFunc( FuncDef( TYPE(Int),    "@<=@", TYPE(Int)   , TYPE(Int)    ) );
//     funcList.addFunc( FuncDef( TYPE(Double), "@<=@", TYPE(Double), TYPE(Double) ) );

//     funcList.addFunc( FuncDef( TYPE(Int),    "++@", TYPE(Int)    ) );
//     funcList.addFunc( FuncDef( TYPE(Double), "++@", TYPE(Double) ) );

//     funcList.addFunc( FuncDef( TYPE(Int),    "@++", TYPE(Int)    ) );
//     funcList.addFunc( FuncDef( TYPE(Double), "@++", TYPE(Double) ) );

//     funcList.addFunc( FuncDef( TYPE(Int),    "--@", TYPE(Int)    ) );
//     funcList.addFunc( FuncDef( TYPE(Double), "--@", TYPE(Double) ) );

//     funcList.addFunc( FuncDef( TYPE(Int),    "@--", TYPE(Int)    ) );
//     funcList.addFunc( FuncDef( TYPE(Double), "@--", TYPE(Double) ) );
}
