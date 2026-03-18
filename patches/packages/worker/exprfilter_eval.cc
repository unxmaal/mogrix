/* exprfilter_eval.cc
 * This file belongs to Worker, a file manager for UN*X/X11.
 * Copyright (C) 2014 Ralf Hoffmann.
 * You can contact me at: ralf@boomerangsworld.de
 *   or http://www.boomerangsworld.de/worker
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "exprfilter_eval.hh"
#include "exprfilter_evalatoms.hh"

void ExprFilterEval::pushAtom( std::shared_ptr< ExprFilterEvalAtom > atom )
{
    m_atoms.push_back( atom );
}

bool ExprFilterEval::popValue()
{
    /* IRIX fix: return false on empty stack instead of throwing.
       DWARF unwinder crashes on IRIX libpthread frames. */
    if ( m_eval_values.empty() ) return false;

    bool v = m_eval_values.back();
    m_eval_values.pop_back();

    return v;
}

void ExprFilterEval::pushValue( bool v )
{
    m_eval_values.push_back( v );
}

bool ExprFilterEval::eval( NWCEntrySelectionState &element )
{
    m_eval_values.clear();
    m_error_occured = false;
        
    for ( auto &a : m_atoms ) {
        if ( a ) {
            a->eval( element, *this );
        }

        if ( m_error_occured ) break;
    }

    return popValue();
}

void ExprFilterEval::setErrorOccured()
{
    m_error_occured = true;
}

bool ExprFilterEval::getErrorOccured() const
{
    return m_error_occured;
}
