/*
 * Illarionserver - server for the game Illarion
 * Copyright 2011 Illarion e.V.
 *
 * This file is part of Illarionserver.
 *
 * Illarionserver  is  free  software:  you can redistribute it and/or modify it
 * under the terms of the  GNU  General  Public License as published by the Free
 * Software Foundation, either version 3 of the License, or (at your option) any
 * later version.
 *
 * Illarionserver is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY;  without  even  the  implied  warranty  of  MERCHANTABILITY  or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more
 * details.
 *
 * You should have received a copy of the GNU  General Public License along with
 * Illarionserver. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _INPUT_DIALOG_HPP_
#define _INPUT_DIALOG_HPP_

#include "dialog/Dialog.hpp"

class InputDialog: public Dialog {
private:
    string description;
    bool  multiline;
    unsigned short maxChars;
    bool success;
    string input;

public:
    InputDialog(const string &title, const string &description, bool multiline, unsigned short maxChars, const luabind::object &callback);
    InputDialog(const InputDialog &inputDialog);
    const string &getDescription() const;
    bool isMultiline() const;
    unsigned short getMaxChars() const;
    bool getSuccess() const;
    void setSuccess(bool success);
    const string &getInput() const;
    void setInput(string input);
};

#endif

