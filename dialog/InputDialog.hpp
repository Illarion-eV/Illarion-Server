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
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU  General Public License along with
 * Illarionserver. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _INPUT_DIALOG_HPP_
#define _INPUT_DIALOG_HPP_

#include "dialog/Dialog.hpp"
#include <string>

class InputDialog: public Dialog {
private:
    std::string title;
    bool  multiline;
    unsigned short maxChars;
    std::string input;

public:
    InputDialog(std::string title, bool multiline, unsigned short maxChars);
    InputDialog(const InputDialog& inputDialog);
    std::string getTitle() const;
    bool isMultiline() const;
    unsigned short getMaxChars() const;
    std::string getInput() const;
    void setInput(std::string input);
};

#endif

