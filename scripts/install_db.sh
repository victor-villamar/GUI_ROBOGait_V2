#!/bin/bash

#############################################################################
#                                                                           #
# ROBOGait Database Installation Script                                     #
#                                                                           #
# This script sets up the database in /opt/robogait with proper permissions #
# for multi-user access.                                                    #
#                                                                           #
# Usage: sudo ./install_db.sh                                               #
#                                                                           #
#############################################################################

set -e  # Exit on error

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
RESET='\033[0m' # No Color

# Configuration
DB_DIR="/opt/robogait"
DB_FILE="$DB_DIR/db_robogait.db"
GROUP_NAME="robogait"
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
SOURCE_DB="$SCRIPT_DIR/../db_robogait.db"

# Check if running as root
if [[ $EUID -ne 0 ]]; then
   echo -e "${RED}ERROR: This script must be run as root (use sudo)${RESET}"
   exit 1
fi

# Get the username from SUDO_USER
TARGET_USER="${SUDO_USER}"

if [ -z "$TARGET_USER" ]; then
    echo -e "${RED}ERROR: Could not determine user. Run with: sudo -u username ./install_db.sh${RESET}"
    exit 1
fi

echo -e "${GREEN}=== ROBOGait Database Installation ===${RESET}"
echo "Target directory: $DB_DIR"
echo "Target user: $TARGET_USER"
echo ""

# Check if group exists
if ! getent group "$GROUP_NAME" > /dev/null 2>&1; then
    echo -e "${YELLOW}Step 1${RESET} Creating group '$GROUP_NAME'..."
    groupadd "$GROUP_NAME"
    echo -e "${GREEN}      Group created${RESET}"
else
    echo -e "${YELLOW}Step 1${RESET} Group '$GROUP_NAME' already exists"
fi

# Add user to robogait group
echo -e "${YELLOW}Step 2${RESET} Adding user '$TARGET_USER' to group '$GROUP_NAME'..."
if id -nG "$TARGET_USER" | grep -qw "$GROUP_NAME"; then
    echo -e "${GREEN}      User already in group${RESET}"
else
    usermod -aG "$GROUP_NAME" "$TARGET_USER"
    echo -e "${GREEN}      User added to group${RESET}"
    echo -e "${YELLOW}      User '$TARGET_USER' needs to log out and log back in for group changes to take effect${RESET}"
fi

# Create database directory
echo -e "${YELLOW}Step 3${RESET} Creating directory '$DB_DIR'..."
mkdir -p "$DB_DIR"
echo -e "${GREEN}      Directory created${RESET}"

if [ ! -f "$SOURCE_DB" ]; then
    echo -e "${RED}ERROR: Source database not found at: $SOURCE_DB${RESET}"
    exit 1
fi

# Check if database file exists
if [ -f "$DB_FILE" ]; then
    echo -e "${YELLOW}Step 4${RESET} Database already exists at '$DB_FILE'"
    read -p "      Do you want to overwrite it? (y/N): " -n 1 -r
    echo
    if [[ ! $REPLY =~ ^[Yy]$ ]]; then
        echo -e "${YELLOW}      Skipping database copy${RESET}"
    else
        echo -e "${YELLOW}      Copying database template...${RESET}"
        cp "$SOURCE_DB" "$DB_FILE"
        echo -e "${GREEN}      Database copied${RESET}"
    fi
else
    echo -e "${YELLOW}Step 4${RESET} Copying database template..."
    cp "$SOURCE_DB" "$DB_FILE"
    echo -e "${GREEN}      Database copied${RESET}"
fi

# Set permissions
echo -e "${YELLOW}Step 5${RESET} Setting permissions..."
chown -R root:"$GROUP_NAME" "$DB_DIR"
chmod 775 "$DB_DIR"
chmod 664 "$DB_FILE"
echo -e "${GREEN}      Permissions set${RESET}"
echo -e "      Directory: 775 (rwxrwxr-x)"
echo -e "      Database:  664 (rw-rw-r--)"

echo ""
echo -e "${GREEN}=== Installation Complete ===${RESET}"
echo ""
echo -e "${YELLOW}IMPORTANT:${RESET}"
echo -e "  • User '$TARGET_USER' has been added to group '$GROUP_NAME'"
echo -e "  • The user needs to ${YELLOW}log out and log back in${RESET} for group changes to take effect"
echo -e "${RED}You MUST restart your computer for group changes to take effect!${RESET}"
echo -e "  • After restarting, verify with: ${GREEN}groups${RESET}"
echo -e "     (should show 'robogait')"
echo ""
echo -e "Database location: ${GREEN}$DB_FILE${RESET}"
echo ""
