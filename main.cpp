#include <iostream>
#include <string>
#include "a.pb.h"

#include <memory>

#include <google/protobuf/descriptor.h>
#include <google/protobuf/descriptor.pb.h>
#include <google/protobuf/dynamic_message.h>

#define ENABLE_PROFILING
#include "profile.h"
#include "test_runner.h"


namespace GP = google::protobuf;

//#define ENABLE_DEBUG
#ifdef ENABLE_DEBUG
#define DEBUG(x) std::cerr << ">>> DEBUG <" << __FILE__ << ":" << __LINE__ << ">:\n" << (x) << std::endl
#else
#define DEBUG(x)
#endif

constexpr size_t MILLION = 1000000;


GP::FileDescriptorProto* constructDescription() {
    auto file = new GP::FileDescriptorProto;
    file->set_name("gen_a");
    file->set_syntax("proto3");

    {
        auto msg = file->add_message_type();
        msg->set_name("GPoint");
        {
            auto field = msg->add_field();
            field->set_name("abscissa");
            field->set_number(1);
            field->set_type(GP::FieldDescriptorProto::Type::FieldDescriptorProto_Type_TYPE_SINT64);
        }
        {
            auto field = msg->add_field();
            field->set_name("ordinate");
            field->set_number(2);
            field->set_type(GP::FieldDescriptorProto::Type::FieldDescriptorProto_Type_TYPE_SINT64);
        }
    }

    {
        auto msg = file->add_message_type();
        msg->set_name("GRectangle");
        {
            auto field = msg->add_field();
            field->set_name("a");
            field->set_number(1);
            field->set_type(GP::FieldDescriptorProto::Type::FieldDescriptorProto_Type_TYPE_MESSAGE);
            field->set_type_name("GPoint");
        }
        {
            auto field = msg->add_field();
            field->set_name("b");
            field->set_number(2);
            field->set_type(GP::FieldDescriptorProto::Type::FieldDescriptorProto_Type_TYPE_MESSAGE);
            field->set_type_name("GPoint");
        }
        {
            auto field = msg->add_field();
            field->set_name("c");
            field->set_number(3);
            field->set_type(GP::FieldDescriptorProto::Type::FieldDescriptorProto_Type_TYPE_MESSAGE);
            field->set_type_name("GPoint");
        }
        {
            auto field = msg->add_field();
            field->set_name("d");
            field->set_number(4);
            field->set_type(GP::FieldDescriptorProto::Type::FieldDescriptorProto_Type_TYPE_MESSAGE);
            field->set_type_name("GPoint");
        }

        {
            auto ecolor = msg->add_enum_type();
            ecolor->set_name("GColor");
            {
               auto value = ecolor->add_value();
               value->set_name("EMPTY");
               value->set_number(0);
            }
            {
               auto value = ecolor->add_value();
               value->set_name("RED");
               value->set_number(1);
            }
            {
               auto value = ecolor->add_value();
               value->set_name("GREEN");
               value->set_number(2);
            }
            {
               auto value = ecolor->add_value();
               value->set_name("BLUE");
               value->set_number(3);
            }
        }

        {
            auto field = msg->add_field();
            field->set_name("name");
            field->set_number(5);
            field->set_type(GP::FieldDescriptorProto::Type::FieldDescriptorProto_Type_TYPE_STRING);
        }
        {
            auto field = msg->add_field();
            field->set_name("fill");
            field->set_number(6);
            field->set_type(GP::FieldDescriptorProto::Type::FieldDescriptorProto_Type_TYPE_ENUM);
            field->set_type_name("GColor");
        }
        {
            auto field = msg->add_field();
            field->set_name("label");
            field->set_number(7);
            field->set_type(GP::FieldDescriptorProto::Type::FieldDescriptorProto_Type_TYPE_STRING);
        }
    }

    return file;
}


std::string getRectangleBinary(
        std::string name,
        int64_t x1, int64_t x2, int64_t y1, int64_t y2,
        std::string label,
        TRectangle::EColor color) {

    TRectangle msg;

    msg.mutable_a()->set_abscissa(x1);
    msg.mutable_a()->set_ordinate(y1);
    msg.mutable_b()->set_abscissa(x1);
    msg.mutable_b()->set_ordinate(y2);
    msg.mutable_c()->set_abscissa(x2);
    msg.mutable_c()->set_ordinate(y2);
    msg.mutable_d()->set_abscissa(x2);
    msg.mutable_d()->set_ordinate(y1);

    msg.set_fill(color);
    msg.set_label(label);
    msg.set_name(name);

    return msg.SerializeAsString();
}


#define REPEAT(x) (x); (x); (x); (x); (x); (x); (x); (x); (x); (x);

void testCompiletimeMessage() {
    auto binary = getRectangleBinary("red_square", 10010, 10020, 10030, 10040, "The Big Red Квадрат", TRectangle_EColor_RED);
    DEBUG(TRectangle::descriptor()->file()->DebugString());

    TRectangle message;

    {
        LOG_DURATION("Compiled message, parse");
        for (size_t j = 0; j < MILLION; ++j) {
            message.ParseFromString(binary);
        }
    }

    {
        int64_t abscissa;
        {
            LOG_DURATION("Compiled message, access int (x10)");
            for (size_t j = 0; j < MILLION; ++j) {
                REPEAT(message.a().abscissa());
            }
        }
        abscissa = message.a().abscissa();
        ASSERT_EQUAL(abscissa, 10010);
    }

    {
        std::string name;
        LOG_DURATION("Compiled message, access string");
        for (size_t j = 0; j < MILLION; ++j) {
            name = message.name();
        }
        ASSERT_EQUAL(name, "red_square");

    }

    {
        LOG_DURATION("Compiled message, serialize");
        for (size_t j = 0; j < MILLION; ++j) {
            message.SerializeAsString();
        }
    }

    DEBUG(message.DebugString());
}


void testRuntimeMessage() {
    auto binary = getRectangleBinary("red_square", 10010, 10020, 10030, 10040, "The Big Red Квадрат", TRectangle_EColor_RED);

    GP::DescriptorPool pool;
    auto file = pool.BuildFile(*constructDescription());
    DEBUG(file->DebugString());

    GP::DynamicMessageFactory factory;
    auto message_prototype = factory.GetPrototype(file->FindMessageTypeByName("GRectangle"));
    auto point_prototype = factory.GetPrototype(file->FindMessageTypeByName("GPoint"));

    auto message = message_prototype->New();

    {
        LOG_DURATION("Runtime message, parse");
        for (size_t j = 0; j < MILLION; ++j) {
            message->ParseFromString(binary);
        }
    }

    auto rect_dsc = message->GetDescriptor();
    DEBUG(rect_dsc != nullptr);
    auto a_dsc = rect_dsc->FindFieldByName("a");
    DEBUG(a_dsc != nullptr);
    auto point_dsc = a_dsc->message_type();
    DEBUG(point_dsc != nullptr);
    auto abscissa_dsc = point_dsc->FindFieldByName("abscissa");
    DEBUG(abscissa_dsc != nullptr);
    auto name_dsc = rect_dsc->FindFieldByName("name");
    DEBUG(name_dsc != nullptr);
    auto rect_refl = message_prototype->GetReflection();
    DEBUG(rect_refl != nullptr);
    auto point_refl = point_prototype->GetReflection();
    DEBUG(rect_refl != nullptr);

    {
        int64_t abscissa;
        {
            LOG_DURATION("Runtime message, access int (x10)");
            for (size_t j = 0; j < MILLION; ++j) {
                REPEAT(point_refl->GetInt64(rect_refl->GetMessage(*message, a_dsc), abscissa_dsc));
            }
        }
        abscissa = point_refl->GetInt64(rect_refl->GetMessage(*message, a_dsc), abscissa_dsc);
        ASSERT_EQUAL(abscissa, 10010);
    }

    {
        std::string name;
        {
            LOG_DURATION("Runtime message, access string");
            for (size_t j = 0; j < MILLION; ++j) {
                name = message->GetReflection()->GetString(*message, name_dsc);
            }
        }
        ASSERT_EQUAL(name, "red_square");
    }

    {
        LOG_DURATION("Runtime message, serialize");
        for (size_t j = 0; j < MILLION; ++j) {
            message->SerializeAsString();
        }
    }

    DEBUG(message->DebugString());
}


int main() {
    TestRunner tr;
    RUN_TEST(tr, testCompiletimeMessage);
    RUN_TEST(tr, testRuntimeMessage);

    return 0;
}
