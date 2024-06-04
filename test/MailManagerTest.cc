#include <gtest/gtest.h>
#include "../lib/MailManager.h"


TEST(MailManagerTest, IsEmailValid_ValidEmail_ReturnsTrue) {
    MailManager mailManager;
    std::string validEmail = "test@example.com";
    bool isValid = mailManager.isEmailValid(validEmail);
    EXPECT_TRUE(isValid);
}

TEST(MailManagerTest, IsEmailValid_InvalidEmail_ReturnsFalse) {
    MailManager mailManager;
    std::string invalidEmail = "invalid_email";
    bool isValid = mailManager.isEmailValid(invalidEmail);
    EXPECT_FALSE(isValid);
}
