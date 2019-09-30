// This file is part of TON Key Generator,
// a desktop application for the TON Blockchain project.
//
// For license and copyright information please follow this link:
// https://github.com/ton-blockchain/tonkeygen/blob/master/LEGAL
//
#include "keygen/phrases.h"

namespace tr {

const phrase lng_window_title = { "TON Key Generator" };

const phrase lng_intro_title = { "TON Key Generator" };
const phrase lng_intro_description = { "Create public and private keys for your TON account." };
const phrase lng_intro_next = { "Start" };
const phrase lng_intro_verify = { "Verify existing key" };
const phrase lng_intro_verify_title = { "Verify existing key" };
const phrase lng_intro_verify_text = { "If you have already generated a private key, you can check its validity and get the corresponding public key here." };
const phrase lng_intro_verify_ok = { "Enter words" };
const phrase lng_intro_verify_cancel = { "Cancel" };

const phrase lng_random_seed_title = { "Enter random characters" };
const phrase lng_random_seed_description = { "Press random buttons on your keyboard at least **50 times** to\nimprove the quality of the key generation process." };
const phrase lng_random_seed_amount = { "Characters entered" };
const phrase lng_random_seed_continue = { "Keep pressing random buttons on your keyboard to improve the\nquality of the key generation process." };
const phrase lng_random_seed_ready_total = { "{ready}/{total} characters entered" };
const phrase lng_random_seed_next = { "Generate keys" };

const phrase lng_created_title = { "Keys created" };
const phrase lng_created_description = { "Your TON encryption keys have been generated successfully.\nPlease prepare a piece of paper and a pen to write down the\nrepresentation of your private key." };
const phrase lng_created_next = { "Continue" };

const phrase lng_view_title = { "Your private key" };
const phrase lng_view_description = { "Write down these 24 words in this exact order and keep them in\na secure place. Do not share this list with anyone. If you lose it,\nyou will irrevocably lose access to your TON account." };
const phrase lng_view_next = { "Continue" };

const phrase lng_check_title = { "Verification" };
const phrase lng_check_description = { "Please enter the words you just wrote down to make sure\nyou did it right." };
const phrase lng_check_next = { "Continue" };
const phrase lng_check_bad_title = { "Incorrect words" };
const phrase lng_check_bad_text = { "The words you entered are not\nthe same as in the original list on the\nprevious page." };
const phrase lng_check_bad_view_words = { "View words" };
const phrase lng_check_bad_try_again = { "Try again" };
const phrase lng_check_good_title = { "Well done" };
const phrase lng_check_good_text = { "The words are correct. Please make\nsure you don't lose this list and never\nshare it with anyone." };
const phrase lng_check_good_next = { "Continue" };

const phrase lng_verify_title = { "Verify existing key" };
const phrase lng_verify_description = { "Enter the secret words corresponding to your private key to validate it." };
const phrase lng_verify_bad_title = { "Incorrect words" };
const phrase lng_verify_bad_text = { "The secret words you have entered do not correspond to any public key." };
const phrase lng_verify_bad_try_again = { "Try again" };
const phrase lng_verify_good_title = { "Well done" };
const phrase lng_verify_good_text = { "The words are correct. Please make\nsure you don't lose this list and never\nshare it with anyone." };
const phrase lng_verify_good_next = { "View public key" };

const phrase lng_done_title = { "Your public key" };
#ifdef KEYGEN_OFFICIAL_BUILD
#include "../../../DesktopPrivate/tonkeygen_official_phrases.h"
#else // KEYGEN_OFFICIAL_BUILD
const phrase lng_done_description = { "This is your public key. To gain access to your TON account,\nplease send this key to the email address from which\nyou received the link to this software." };
#endif // KEYGEN_OFFICIAL_BUILD
const phrase lng_done_copy_key = { "Copy public key" };
const phrase lng_done_save_key = { "Save as file" };
const phrase lng_done_verify_key = { "Verify private key" };
const phrase lng_done_generate_new = { "Generate new key" };
const phrase lng_done_to_clipboard = { "Public key copied to clipboard." };
const phrase lng_done_save_caption = { "Choose file name" };
const phrase lng_done_to_file = { "Public key saved to file." };
const phrase lng_done_new_title = { "Are you sure?" };
const phrase lng_done_new_text = { "Generating a new pair of keys makes\nsense only if you lost your list of 24\nsecret words and haven't sent your\npublic key to TON developers yet.\n\nIf you want to generate another key for a multisignature wallet, please use a different computer. Do not generate more than one key on the same machine." };
const phrase lng_done_new_cancel = { "Cancel" };
const phrase lng_done_new_ok = { "OK" };

} // namespace tr
