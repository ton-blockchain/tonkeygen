// This file is part of TON Key Generator,
// a desktop application for the TON Blockchain project.
//
// For license and copyright information please follow this link:
// https://github.com/ton-blockchain/tonkeygen/blob/master/LEGAL
//
#include "keygen/phrases.h"

namespace tr {

phrase lng_window_title = { "TON Key Generator" };

phrase lng_intro_title = { "TON Key Generator" };
phrase lng_intro_description = { "Create public and private keys for your TON account." };
phrase lng_intro_next = { "Start" };

phrase lng_random_seed_title = { "Enter random characters" };
phrase lng_random_seed_description = { "Press random buttons on your keyboard at least **50 times** to\nimprove the quality of the key generation process." };
phrase lng_random_seed_amount = { "Characters entered" };
phrase lng_random_seed_continue = { "Keep pressing random buttons on your keyboard to improve the\nquality of the key generation process." };
phrase lng_random_seed_ready_total = { "{ready}/{total} characters entered" };
phrase lng_random_seed_next = { "Generate keys" };

phrase lng_created_title = { "Keys created" };
phrase lng_created_description = { "Your TON encryption keys have been generated successfully.\nPlease prepare a piece of paper and a pen to write down the\nrepresentation of your private key." };
phrase lng_created_next = { "Continue" };

phrase lng_view_title = { "Your private key" };
phrase lng_view_description = { "Write down these 24 words in the exact order and keep them in\na secure place. Do not share this list with anyone. If you lose it,\nyou will irrevocably lose access to your TON account." };
phrase lng_view_next = { "Continue" };

phrase lng_check_title = { "Verification" };
phrase lng_check_description = { "Please enter the words you've just written down to make sure\nyou've done it right." };
phrase lng_check_next = { "Continue" };
phrase lng_check_bad_title = { "Incorrect words" };
phrase lng_check_bad_text = { "The words you have entered are not\nthe same as in the original list on the\nprevious page." };
phrase lng_check_bad_view_words = { "View words" };
phrase lng_check_bad_try_again = { "Try again" };
phrase lng_check_good_title = { "Well done" };
phrase lng_check_good_text = { "The words are correct. Please make\nsure you don't lose this list and not\nshare it with anyone." };
phrase lng_check_good_next = { "Continue" };

phrase lng_done_title = { "Your public key" };
phrase lng_done_description = { "Below is your public key. Please save it and share it with TON\ndevelopers to gain access to your TON account." };
phrase lng_done_copy_key = { "Copy public key" };
phrase lng_done_save_key = { "Save as file" };
phrase lng_done_verify_key = { "Verify private key" };
phrase lng_done_generate_new = { "Generate new key" };
phrase lng_done_to_clipboard = { "Public key copied to clipboard." };
phrase lng_done_save_caption = { "Choose file name" };
phrase lng_done_to_file = { "Public key saved to file." };
phrase lng_done_new_title = { "Are you sure?" };
phrase lng_done_new_text = { "Generating new pair of keys makes\nsense only if you've lost the list of 24\nsecret words and haven't yet sent your\npublic key to TON developers." };
phrase lng_done_new_cancel = { "Cancel" };
phrase lng_done_new_ok = { "OK" };

} // namespace tr
