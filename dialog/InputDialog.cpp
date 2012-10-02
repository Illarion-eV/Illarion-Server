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

#include "dialog/InputDialog.hpp"

InputDialog::InputDialog(std::string title, std::string description, bool multiline,
                         unsigned short maxChars, luabind::object callback)
    :Dialog(title, "InputDialog", callback), description(description), multiline(multiline), maxChars(maxChars) {
    success = false;
    input = "";
}

InputDialog::InputDialog(const InputDialog &inputDialog) : Dialog(inputDialog) {
    description = inputDialog.description;
    multiline = inputDialog.multiline;
    maxChars = inputDialog.maxChars;
    success = inputDialog.success;
    input = inputDialog.input;
}

std::string InputDialog::getDescription() const {
    return description;
}

bool InputDialog::isMultiline() const {
    return multiline;
}

unsigned short InputDialog::getMaxChars() const {
    return maxChars;
}

bool InputDialog::getSuccess() const {
    return success;
}

void InputDialog::setSuccess(bool success) {
    this->success = success;
}

std::string InputDialog::getInput() const {
    return input;
}

void InputDialog::setInput(std::string input) {
    if (maxChars > 0) {
        input = input.substr(0, maxChars);
    }

    if (!multiline) {
        input = input.substr(0, input.find('\n', 0));
    }

    this->input = input;
}

